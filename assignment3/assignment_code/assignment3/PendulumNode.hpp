#ifndef PENDULUM_NODE_H_
#define PENDULUM_NODE_H_

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"

#include "ParticleState.hpp"
#include "IntegratorFactory.hpp"
#include "PendulumSystem.hpp"

namespace GLOO {
class PendulumNode : public SceneNode {
    public:
        PendulumNode(IntegratorType type, float step_size) {
            sphere_mesh_ = PrimitiveFactory::CreateSphere(0.05f, 20, 20);
            shader_ = std::make_shared<PhongShader>();

            state_ = ParticleState();
            system_ = PendulumSystem();

            auto sphere_node_one = make_unique<SceneNode>();
            sphere_node_one->GetTransform().SetPosition(glm::vec3(0, 0, 0));
            state_.positions.push_back(glm::vec3(0, 0, 0));
            state_.velocities.push_back(glm::vec3(0, 0, 0));
            system_.AddMass(0.5f);
            sphere_nodes_.push_back(sphere_node_one.get());
            AddChild(std::move(sphere_node_one));

            auto sphere_node_two = make_unique<SceneNode>();
            sphere_node_two->GetTransform().SetPosition(glm::vec3(0, 1, -1));
            state_.positions.push_back(glm::vec3(0, 1, -1));
            state_.velocities.push_back(glm::vec3(0, 0, 0));
            system_.AddMass(0.005f);
            system_.AddSpring(0, 1, 0.5f, 0.5f);
            sphere_nodes_.push_back(sphere_node_two.get());
            AddChild(std::move(sphere_node_two));

            auto sphere_node_three = make_unique<SceneNode>();
            sphere_node_three->GetTransform().SetPosition(glm::vec3(0, -1, 1));
            state_.positions.push_back(glm::vec3(0, -1, 1));
            state_.velocities.push_back(glm::vec3(0, 0, 0));
            system_.AddMass(0.005f);
            system_.AddSpring(1, 2, 0.5f, 0.5f);
            sphere_nodes_.push_back(sphere_node_three.get());
            AddChild(std::move(sphere_node_three));

            auto sphere_node_four = make_unique<SceneNode>();
            sphere_node_four->GetTransform().SetPosition(glm::vec3(0, 2, 0));
            state_.positions.push_back(glm::vec3(0, 2, 0));
            state_.velocities.push_back(glm::vec3(0, 1, 0));
            system_.AddMass(0.005f);
            system_.AddSpring(2, 3, 0.5f, 0.5f);
            sphere_nodes_.push_back(sphere_node_four.get());
            AddChild(std::move(sphere_node_four));

            for (int i = 0; i < sphere_nodes_.size(); i++) {
                sphere_nodes_[i]->CreateComponent<ShadingComponent>(shader_);
                sphere_nodes_[i]->CreateComponent<RenderingComponent>(sphere_mesh_);
                sphere_nodes_[i]->CreateComponent<MaterialComponent>(std::make_shared<Material>(Material::GetDefault()));
                auto material_component = sphere_nodes_[i]->GetComponentPtr<MaterialComponent>();
                Material& material = material_component->GetMaterial();
                material.SetDiffuseColor(glm::vec3(0.0f, 1.0f, 0.0f));
                material.SetAmbientColor(glm::vec3(0.0f, 1.0f, 0.0f));
            }

            system_.FixMass(0);

            type_ = type;
            integrator_ = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(type_);
            step_size_ = step_size;
            time_ = 0.0;
        }

        void Update(double delta_time) override {
            for (int i = 0; i < delta_time / step_size_; i++) {
                state_ = integrator_->Integrate(system_, state_, time_, step_size_);
                for (int j = 0; j < sphere_nodes_.size(); j++) {
                    sphere_nodes_[j]->GetTransform().SetPosition(state_.positions[j]);
                }
                time_ += step_size_;
            }
        }

    private:
        ParticleState state_;
        std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator_;
        PendulumSystem system_;
        IntegratorType type_;
        float step_size_;
        float time_;
        
        std::vector<SceneNode*> sphere_nodes_;
        std::shared_ptr<VertexObject> sphere_mesh_;
        std::shared_ptr<ShaderProgram> shader_;
};
}  // namespace GLOO

#endif
