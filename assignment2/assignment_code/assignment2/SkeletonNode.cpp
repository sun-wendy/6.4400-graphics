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
  cylinder_mesh_ = PrimitiveFactory::CreateCylinder(0.01f, 1.0f, 25);

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
}

void SkeletonNode::DecorateTree() {
  // TODO: set up addtional nodes, add necessary components here.
  // You should create one set of nodes/components for skeleton mode
  // (spheres for joints and cylinders for bones), and another set for
  // SSD node (you could just use a single node with a RenderingComponent
  // that is linked to a VertexObject with the mesh information. Then you
  // only need to update the VertexObject - updating vertex positions and
  // recalculating the normals, etc.).

  // The code snippet below shows how to add a sphere node to a joint.
  // Suppose you have created member variables shader_ of type
  // std::shared_ptr<PhongShader>, and sphere_mesh_ of type
  // std::shared_ptr<VertexObject>.
  // Here sphere_nodes_ptrs_ is a std::vector<SceneNode*> that stores the
  // pointer so the sphere nodes can be accessed later to change their
  // positions. joint_ptr is assumed to be one of the joint node you created
  // from LoadSkeletonFile (e.g. you've stored a std::vector<SceneNode*> of
  // joint nodes as a member variable and joint_ptr is one of the elements).
  //
  // auto sphere_node = make_unique<SceneNode>();
  // sphere_node->CreateComponent<ShadingComponent>(shader_);
  // sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
  // sphere_nodes_ptrs_.push_back(sphere_node.get());
  // joint_ptr->AddChild(std::move(sphere_node));

  for (auto joint : joint_nodes_) {
    auto sphere_node = make_unique<SceneNode>();
    sphere_node->CreateComponent<ShadingComponent>(shader_);
    sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    sphere_nodes_ptrs_.push_back(sphere_node.get());
    joint->AddChild(std::move(sphere_node));
  }

  glm::vec3 root_pos = joint_nodes_[0]->GetTransform().GetPosition();

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

      joint->AddChild(std::move(cylinder_node));
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

  for (int i = 0; i < linked_angles_.size(); i++) {
    auto joint = joint_nodes_[i];
    auto euler_angle = linked_angles_[i];
    joint->GetTransform().SetRotation(glm::quat(glm::vec3(euler_angle->rx, euler_angle->ry, euler_angle->rz)));
  }
}

void SkeletonNode::LinkRotationControl(const std::vector<EulerAngle*>& angles) {
  linked_angles_ = angles;
}

void SkeletonNode::LoadSkeletonFile(const std::string& path) {
  // TODO: load skeleton file and build the tree of joints.

  std::fstream file;
  file.open(path);

  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return;
  } else {
    std::cout << "Successfully opened file: " << path << std::endl;

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

      // cur_node->GetTransform().SetPosition(joint_nodes_[parent_id]->GetTransform().GetPosition() + glm::vec3(x, y, z));
    }
  }
}

void SkeletonNode::LoadMeshFile(const std::string& filename) {
  std::shared_ptr<VertexObject> vtx_obj =
      MeshLoader::Import(filename).vertex_obj;
  // TODO: store the bind pose mesh in your preferred way.

  
}

void SkeletonNode::LoadAttachmentWeights(const std::string& path) {
  // TODO: load attachment weights.
}

void SkeletonNode::LoadAllFiles(const std::string& prefix) {
  std::string prefix_full = GetAssetDir() + prefix;
  LoadSkeletonFile(prefix_full + ".skel");
  LoadMeshFile(prefix + ".obj");
  LoadAttachmentWeights(prefix_full + ".attach");
}
}  // namespace GLOO
