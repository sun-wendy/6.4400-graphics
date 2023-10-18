#include "SkeletonNode.hpp"

#include "gloo/utils.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"

#include <fstream>

namespace GLOO {
SkeletonNode::SkeletonNode(const std::string& filename)
    : SceneNode(), draw_mode_(DrawMode::Skeleton) {
  
  shader_ = std::make_shared<PhongShader>();
  sphere_mesh_ = PrimitiveFactory::CreateSphere(0.02f, 10, 10);
  cylinder_mesh_ = PrimitiveFactory::CreateCylinder(0.015f, 1.0f, 25);
  skin_mesh_ = std::make_shared<VertexObject>();

  LoadAllFiles(filename);
  DecorateTree();

  // Force initial update.
  OnJointChanged();
}


void SkeletonNode::ToggleDrawMode() {
  draw_mode_ =
      draw_mode_ == DrawMode::Skeleton ? DrawMode::SSD : DrawMode::Skeleton;
  // TODO: implement here toggling between skeleton mode and SSD mode.
  // The current mode is draw_mode_;
  // Hint: you may find SceneNode::SetActive convenient here as
  // inactive nodes will not be picked up by the renderer.

  if (draw_mode_ == DrawMode::Skeleton) {
    for (auto sphere_node : sphere_nodes_ptrs_) {
      sphere_node->SetActive(true);
    }
    for (auto cylinder_node : cylinder_nodes_ptrs_) {
      cylinder_node->SetActive(true);
    }
    for (auto skin_node : skin_nodes_ptrs_) {
      skin_node->SetActive(false);
    }
  } else {
    for (auto sphere_node : sphere_nodes_ptrs_) {
      sphere_node->SetActive(false);
    }
    for (auto cylinder_node : cylinder_nodes_ptrs_) {
      cylinder_node->SetActive(false);
    }
    for (auto skin_node : skin_nodes_ptrs_) {
      skin_node->SetActive(true);
    }
  }
}


void SkeletonNode::DecorateTree() {
  // TODO: set up addtional nodes, add necessary components here.
  // You should create one set of nodes/components for skeleton mode
  // (spheres for joints and cylinders for bones), and another set for
  // SSD node (you could just use a single node with a RenderingComponent
  // that is linked to a VertexObject with the mesh information. Then you
  // only need to update the VertexObject - updating vertex positions and
  // recalculating the normals, etc.).

  // Draw joints
  for (auto joint : joint_nodes_) {
    auto sphere_node = make_unique<SceneNode>();
    sphere_node->CreateComponent<ShadingComponent>(shader_);
    sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    sphere_nodes_ptrs_.push_back(sphere_node.get());
    joint->AddChild(std::move(sphere_node));
  }

  glm::vec3 root_pos = joint_nodes_[0]->GetTransform().GetPosition();

  // Draw bones
  for (int i = 0; i < joint_nodes_.size(); i++) {
    auto joint = joint_nodes_[i];
    int children_count = joint->GetChildrenCount();
    for (int j = 0; j < children_count; j++) {
      auto& child = joint->GetChild(j);
      auto cylinder_node = make_unique<SceneNode>();
      cylinder_node->CreateComponent<ShadingComponent>(shader_);
      cylinder_node->CreateComponent<RenderingComponent>(cylinder_mesh_);

      // Set position
      cylinder_node->GetTransform().SetPosition(glm::vec3(0, 0, 0));
      
      // Set rotation
      glm::vec3 y_basis(0, 1.0f, 0);
      glm::vec3 child_pos = child.GetTransform().GetPosition();
      glm::vec3 axis = glm::normalize(glm::cross(y_basis, child_pos));
      float angle = glm::acos(glm::dot(y_basis, child_pos) / glm::length(child_pos));
      cylinder_node->GetTransform().SetRotation(axis, angle);
      // glm::mat4 parent_to_local_mat = glm::inverse(joint->GetTransform().GetLocalToParentMatrix());
      // glm::vec3 parent_pos = parent_to_local_mat * glm::vec4(glm::vec3(0), 1);
      // glm::vec3 y = glm::normalize(parent_pos);
      // glm::vec3 axis = glm::normalize(glm::cross(y_basis, y));
      // float angle = glm::acos(glm::dot(y, y_basis));

      // Set scale
      float length = glm::distance(glm::vec3(0, 0, 0), child_pos);
      cylinder_node->GetTransform().SetScale(glm::vec3(1.0f, length, 1.0f));

      cylinder_nodes_ptrs_.push_back(cylinder_node.get());
      joint->AddChild(std::move(cylinder_node));
    }
  }

  // Calculate B_i^{-1} for each joint (only need to calculate once)
  for (int i = 0; i < joint_nodes_.size(); i++) {
    glm::mat4 B_i = joint_nodes_[i]->GetTransform().GetLocalToWorldMatrix();
    glm::mat4 inv_B_i = glm::inverse(B_i);
    inv_bind_transform_.push_back(inv_B_i);
  }

  // Calculate normals
  auto final_normals = make_unique<NormalArray>();
  std::vector<glm::vec3> normals;
  std::vector<float> vtx_weights(bind_vertices_.size(), 0.0f);
  auto indices = skin_mesh_->GetIndices();

  for (auto vertex : bind_vertices_) {
    normals.push_back(glm::vec3(0, 0, 0));
  }

  for (size_t i = 0; i < indices.size() - 2; i += 3) {
    int v1 = indices[i];
    int v2 = indices[i+2];
    int v3 = indices[i+1];

    auto e1_e2 = glm::cross(bind_vertices_[v1] - bind_vertices_[v2], bind_vertices_[v3] - bind_vertices_[v2]);
    float length = glm::length(e1_e2);
    glm::vec3 face_normal = e1_e2 / length;
    float face_weight = 0.5 * length;

    vtx_weights[v1] += face_weight;
    vtx_weights[v2] += face_weight;
    vtx_weights[v3] += face_weight;

    normals[v1] += face_weight * face_normal;
    normals[v2] += face_weight * face_normal;
    normals[v3] += face_weight * face_normal;
  }

  for (size_t j = 0; j < normals.size(); j++) {
    normals[j] /= vtx_weights[j];
  }

  for (auto single_normal : normals) {
    final_normals->push_back(single_normal);
  }
  skin_mesh_->UpdateNormals(std::move(final_normals));

  // Draw skin mesh
  auto skin_node = make_unique<SceneNode>();
  skin_node->CreateComponent<ShadingComponent>(shader_);
  skin_node->CreateComponent<RenderingComponent>(skin_mesh_);
  skin_nodes_ptrs_.push_back(skin_node.get());
  AddChild(std::move(skin_node));

  // Draw mode initialization
  if (draw_mode_ == DrawMode::Skeleton) {
    for (auto sphere_node : sphere_nodes_ptrs_) {
      sphere_node->SetActive(true);
    }
    for (auto cylinder_node : cylinder_nodes_ptrs_) {
      cylinder_node->SetActive(true);
    }
    for (auto skin_node : skin_nodes_ptrs_) {
      skin_node->SetActive(false);
    }
  } else {
    for (auto sphere_node : sphere_nodes_ptrs_) {
      sphere_node->SetActive(false);
    }
    for (auto cylinder_node : cylinder_nodes_ptrs_) {
      cylinder_node->SetActive(false);
    }
    for (auto skin_node : skin_nodes_ptrs_) {
      skin_node->SetActive(true);
    }
  }
}


void SkeletonNode::Update(double delta_time) {
  // Prevent multiple toggle.
  static bool prev_released = true;
  if (InputManager::GetInstance().IsKeyPressed('S')) {
    if (prev_released) {
      ToggleDrawMode();
    }
    prev_released = false;
  } else if (InputManager::GetInstance().IsKeyReleased('S')) {
    prev_released = true;
  }
}


void SkeletonNode::OnJointChanged() {
  // TODO: this method is called whenever the values of UI sliders change.
  // The new Euler angles (represented as EulerAngle struct) can be retrieved
  // from linked_angles_ (a std::vector of EulerAngle*).
  // The indices of linked_angles_ align with the order of the joints in .skel
  // files. For instance, *linked_angles_[0] corresponds to the first line of
  // the .skel file.

  // Update skeleton
  for (int i = 0; i < linked_angles_.size(); i++) {
    auto joint = joint_nodes_[i];
    auto euler_angle = linked_angles_[i];
    joint->GetTransform().SetRotation(glm::quat(glm::vec3(euler_angle->rx, euler_angle->ry, euler_angle->rz)));
  }

  // Update skin mesh
  auto positions = make_unique<PositionArray>();

  for (size_t i = 0; i < bind_vertices_.size(); i++) {
    glm::vec4 p(bind_vertices_[i], 1.0);
    glm::vec4 new_pos(0.0, 0.0, 0.0, 0.0);

    for (size_t j = 0; j < weights_[0].size(); j++) {
      float weight = weights_[i][j];
      glm::mat4 T = joint_nodes_[j+1]->GetTransform().GetLocalToWorldMatrix();
      glm::mat4 inv_B = inv_bind_transform_[j+1];
      new_pos += weight * T * inv_B * p;
    }

    positions->push_back(glm::vec3(new_pos));
  }

  skin_mesh_->UpdatePositions(std::move(positions));

  // Update normals
  auto new_pos = skin_mesh_->GetPositions();
  
  auto final_normals = make_unique<NormalArray>();
  std::vector<glm::vec3> normals;
  std::vector<float> new_vtx_weights(bind_vertices_.size(), 0.0f);
  auto indices = skin_mesh_->GetIndices();

  for (auto vertex : bind_vertices_) {
    normals.push_back(glm::vec3(0, 0, 0));
  }

  for (size_t i = 0; i < indices.size() - 2; i += 3) {
    int v1 = indices[i];
    int v2 = indices[i+2];
    int v3 = indices[i+1];

    auto e1_e2 = glm::cross(new_pos[v1] - new_pos[v2], new_pos[v3] - new_pos[v2]);
    float length = glm::length(e1_e2);
    glm::vec3 face_normal = e1_e2 / length;
    float face_weight = 0.5 * length;

    new_vtx_weights[v1] += face_weight;
    new_vtx_weights[v2] += face_weight;
    new_vtx_weights[v3] += face_weight;

    normals[v1] += face_weight * face_normal;
    normals[v2] += face_weight * face_normal;
    normals[v3] += face_weight * face_normal;
  }

  for (size_t j = 0; j < normals.size(); j++) {
    normals[j] /= new_vtx_weights[j];
  }

  for (auto single_normal : normals) {
    final_normals->push_back(single_normal);
  }
  skin_mesh_->UpdateNormals(std::move(final_normals));
}


void SkeletonNode::LinkRotationControl(const std::vector<EulerAngle*>& angles) {
  linked_angles_ = angles;
}


void SkeletonNode::LoadSkeletonFile(const std::string& path) {
  // TODO: load skeleton file and build the tree of joints.

  std::fstream file;
  file.open(path);

  if (!file.is_open()) {
    std::cerr << "Failed to load skeleton file" << std::endl;
    return;
  } else {
    std::string line;

    while (std::getline(file, line)) {
      std::stringstream ss(line);
      float x, y, z;
      int parent_id;
      ss >> x >> y >> z >> parent_id;

      auto cur_node = make_unique<SceneNode>();
      joint_nodes_.push_back(cur_node.get());
      cur_node->GetTransform().SetPosition(glm::vec3(x, y, z));

      if (parent_id == -1) {
        AddChild(std::move(cur_node));
      } else {
        joint_nodes_[parent_id]->AddChild(std::move(cur_node));
      }
    }
  }
}


void SkeletonNode::LoadMeshFile(const std::string& filename) {
  std::shared_ptr<VertexObject> vtx_obj =
      MeshLoader::Import(filename).vertex_obj;
  // TODO: store the bind pose mesh in your preferred way.

  skin_mesh_ = vtx_obj;
  bind_vertices_ = vtx_obj->GetPositions();
}


void SkeletonNode::LoadAttachmentWeights(const std::string& path) {
  // TODO: load attachment weights.
  
  std::fstream file;
  file.open(path);

  if (!file.is_open()) {
    std::cerr << "Failed to load attachment weights" << std::endl;
    return;
  } else {
    std::string line;

    while (std::getline(file, line)) {
      std::stringstream ss(line);
      std::vector<float> row;
      float single_weight;
      while (ss >> single_weight) {
        row.push_back(single_weight);
      }
      weights_.push_back(row);
    }
  }
}


void SkeletonNode::LoadAllFiles(const std::string& prefix) {
  std::string prefix_full = GetAssetDir() + prefix;
  LoadSkeletonFile(prefix_full + ".skel");
  LoadMeshFile(prefix + ".obj");
  LoadAttachmentWeights(prefix_full + ".attach");
}
}  // namespace GLOO
