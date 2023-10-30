#ifndef CLOTH_NODE_H_
#define CLOTH_NODE_H_

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/InputManager.hpp"

#include "ParticleState.hpp"
#include "IntegratorFactory.hpp"
#include "ClothSystem.hpp"

namespace GLOO {
class ClothNode : public SceneNode {
    public:
        ClothNode(IntegratorType type, float step_size) {
            sphere_mesh_ = PrimitiveFactory::CreateSphere(0.05f, 20, 20);
            shader_ = std::make_shared<PhongShader>();

            state_ = ParticleState();
            system_ = ClothSystem();

            auto cloth_positions = make_unique<PositionArray>();

            for (int j = 0; j < 6; j++) {
                for (int i = 0; i < 6; i++) {
                    auto sphere_node = make_unique<SceneNode>();
                    sphere_node->CreateComponent<ShadingComponent>(shader_);
                    sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
                    sphere_nodes_.push_back(sphere_node.get());
                    AddChild(std::move(sphere_node));
                }
            }

            for (int j = 0; j < 6; j++) {
                for (int i = 0; i < 6; i++) {
                    glm::vec3 pos(j * 0.5, -i * 0.05, i * 0.5);
                    int idx = system_.IndexOf(i, j);
                    sphere_nodes_[idx]->GetTransform().SetPosition(pos);
                    state_.positions.push_back(pos);

                    glm::vec3 vel(0.0f);
                    state_.velocities.push_back(vel);
                    system_.AddMass(0.005f);

                    if (i == 0) {
                        system_.FixMass(idx);
                    }

                    cloth_positions->push_back(pos);
                }
            }

            // Add structural springs
            for (int j = 0; j < 6; j++) {
                for (int i = 0; i < 5; i++) {
                    system_.AddSpring(system_.IndexOf(i, j), system_.IndexOf(i + 1, j), 0.4f, 0.3f);
                    system_.AddSpring(system_.IndexOf(j, i), system_.IndexOf(j, i + 1), 0.4f, 0.3f);
                }
            }

            // Add shear springs
            for (int j = 0; j < 5; j++) {
                for (int i = 0; i < 5; i++) {
                    system_.AddSpring(system_.IndexOf(i, j), system_.IndexOf(i + 1, j + 1), 0.4f, 0.3f);
                    system_.AddSpring(system_.IndexOf(i + 1, j), system_.IndexOf(i, j + 1), 0.4f, 0.3f);
                }
            }

            // Add flex springs
            for (int j = 0; j < 6; j++) {
                for (int i = 0; i < 4; i++) {
                    system_.AddSpring(system_.IndexOf(i, j), system_.IndexOf(i + 2, j), 0.4f, 0.3f);
                    system_.AddSpring(system_.IndexOf(j, i), system_.IndexOf(j, i + 2), 0.4f, 0.3f);
                }
            }

            type_ = type;
            integrator_ = IntegratorFactory::CreateIntegrator<ClothSystem, ParticleState>(type_);

            step_size_ = step_size;
            time_ = 0.0;

            // Render cloth
            cloth_mesh_ = std::make_shared<VertexObject>();
            auto indices = make_unique<IndexArray>();

            for (int j = 0; j < 5; j++) {
                for (int i = 0; i < 5; i++) {
                    int here = system_.IndexOf(i, j);
                    int right = system_.IndexOf(i + 1, j);
                    int down = system_.IndexOf(i, j + 1);
                    int diag = system_.IndexOf(i + 1, j + 1);

                    indices->push_back(down);
                    indices->push_back(diag);
                    indices->push_back(here);
                    indices->push_back(here);
                    indices->push_back(diag);
                    indices->push_back(right);
                }
            }

            cloth_mesh_->UpdatePositions(std::move(cloth_positions));
            cloth_mesh_->UpdateIndices(std::move(indices));

            // Initialize normals
            auto final_normals = make_unique<NormalArray>();
            std::vector<glm::vec3> normals;
            size_t pos_size = cloth_mesh_->GetPositions().size();
            auto new_pos = cloth_mesh_->GetPositions();
            std::vector<float> vtx_weights(pos_size, 0.0f);
            auto cloth_indices = cloth_mesh_->GetIndices();

            for (size_t i = 0; i < pos_size; i++) {
                normals.push_back(glm::vec3(0, 0, 0));
            }

            for (size_t i = 0; i < cloth_indices.size() - 2; i += 3) {
                int v1 = cloth_indices[i];
                int v2 = cloth_indices[i+2];
                int v3 = cloth_indices[i+1];

                auto e1_e2 = glm::cross(new_pos[v1] - new_pos[v2], new_pos[v3] - new_pos[v2]);
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

            for (size_t i = 0; i < normals.size(); i++) {
                normals[i] /= vtx_weights[i];
            }

            for (auto normal : normals) {
                final_normals->push_back(normal);
            }

            cloth_mesh_->UpdateNormals(std::move(final_normals));

            auto cloth_node = make_unique<SceneNode>();
            cloth_node->CreateComponent<ShadingComponent>(shader_);
            auto& rc = cloth_node->CreateComponent<RenderingComponent>(cloth_mesh_);
            rc.SetDrawMode(DrawMode::Triangles);
            cloth_node->CreateComponent<MaterialComponent>(std::make_shared<Material>(Material::GetDefault()));
            AddChild(std::move(cloth_node));
        }


        void Update(double delta_time) override {
            state_ = integrator_->Integrate(system_, state_, time_, step_size_);
            
            // Update sphere pos
            auto cloth_positions = make_unique<PositionArray>();

            for (int i = 0; i < sphere_nodes_.size(); i++) {
                sphere_nodes_[i]->GetTransform().SetPosition(state_.positions[i]);
                cloth_positions->push_back(state_.positions[i]);
            }

            cloth_mesh_->UpdatePositions(std::move(cloth_positions));

            // Update normals
            auto new_normals = make_unique<NormalArray>();
            std::vector<glm::vec3> normals;
            size_t pos_size = cloth_mesh_->GetPositions().size();
            auto new_pos = cloth_mesh_->GetPositions();
            std::vector<float> vtx_weights(pos_size, 0.0f);
            auto cloth_indices = cloth_mesh_->GetIndices();

            for (size_t i = 0; i < pos_size; i++) {
                normals.push_back(glm::vec3(0, 0, 0));
            }

            for (size_t i = 0; i < cloth_indices.size() - 2; i += 3) {
                int v1 = cloth_indices[i];
                int v2 = cloth_indices[i+2];
                int v3 = cloth_indices[i+1];

                auto e1_e2 = glm::cross(new_pos[v1] - new_pos[v2], new_pos[v3] - new_pos[v2]);
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

            for (size_t i = 0; i < normals.size(); i++) {
                normals[i] /= vtx_weights[i];
            }

            for (auto normal : normals) {
                new_normals->push_back(normal);
            }

            cloth_mesh_->UpdateNormals(std::move(new_normals));

            time_ += delta_time;

            // Wire 'R' key to reset cloth
            static bool r_prev_released = true;
            if (InputManager::GetInstance().IsKeyPressed('R')) {
                if (r_prev_released) {
                    Reset();
                }
                r_prev_released = false;
            } else{
                r_prev_released = true;
            }
        }


        void Reset() {
            state_ = ParticleState();
            system_ = ClothSystem();

            auto cloth_positions = make_unique<PositionArray>();

            for (int j = 0; j < 6; j++) {
                for (int i = 0; i < 6; i++) {
                    glm::vec3 pos(j * 0.5, -i * 0.05, i * 0.5);
                    int idx = system_.IndexOf(i, j);
                    sphere_nodes_[idx]->GetTransform().SetPosition(pos);
                    state_.positions.push_back(pos);

                    glm::vec3 vel(0.0f);
                    state_.velocities.push_back(vel);
                    system_.AddMass(0.005f);

                    cloth_positions->push_back(pos);
                }
            }

            // Add structural springs
            for (int j = 0; j < 6; j++) {
                for (int i = 0; i < 5; i++) {
                    system_.AddSpring(system_.IndexOf(i, j), system_.IndexOf(i + 1, j), 0.5f, 0.3f);
                    system_.AddSpring(system_.IndexOf(j, i), system_.IndexOf(j, i + 1), 0.5f, 0.3f);
                }
            }

            // Add shear springs
            for (int j = 0; j < 5; j++) {
                for (int i = 0; i < 5; i++) {
                    system_.AddSpring(system_.IndexOf(i, j), system_.IndexOf(i + 1, j + 1), 0.5f, 0.3f);
                    system_.AddSpring(system_.IndexOf(i + 1, j), system_.IndexOf(i, j + 1), 0.5f, 0.3f);
                }
            }

            // Add flex springs
            for (int j = 0; j < 6; j++) {
                for (int i = 0; i < 4; i++) {
                    system_.AddSpring(system_.IndexOf(i, j), system_.IndexOf(i + 2, j), 0.5f, 0.3f);
                    system_.AddSpring(system_.IndexOf(j, i), system_.IndexOf(j, i + 2), 0.5f, 0.3f);
                }
            }

            cloth_mesh_->UpdatePositions(std::move(cloth_positions));

            time_ = 0.0;
        }

    private:
        ParticleState state_;
        std::unique_ptr<IntegratorBase<ClothSystem, ParticleState>> integrator_;
        ClothSystem system_;
        IntegratorType type_;
        float step_size_;
        float time_;
        
        std::vector<SceneNode*> sphere_nodes_;
        std::shared_ptr<VertexObject> sphere_mesh_;
        std::shared_ptr<ShaderProgram> shader_;

        std::shared_ptr<VertexObject> cloth_mesh_;
};
}  // namespace GLOO

#endif
