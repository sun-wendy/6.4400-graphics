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
            sphere_node_one->CreateComponent<ShadingComponent>(shader_);
            sphere_node_one->CreateComponent<RenderingComponent>(sphere_mesh_);
            sphere_node_one->CreateComponent<MaterialComponent>(std::make_shared<Material>(Material::GetDefault()));
            auto material_component_one = sphere_node_one->GetComponentPtr<MaterialComponent>();
            Material& material_one = material_component_one->GetMaterial();
            material_one.SetDiffuseColor(glm::vec3(0.0f, 1.0f, 0.0f));
            material_one.SetAmbientColor(glm::vec3(0.0f, 1.0f, 0.0f));
            sphere_nodes_.push_back(sphere_node_one.get());
            AddChild(std::move(sphere_node_one));

            auto sphere_node_two = make_unique<SceneNode>();
            sphere_node_two->GetTransform().SetPosition(glm::vec3(0, 1, -1));
            state_.positions.push_back(glm::vec3(0, 1, -1));
            state_.velocities.push_back(glm::vec3(0, 0, 0));
            system_.AddMass(0.005f);
            system_.AddSpring(0, 1, 0.5f, 0.5f);
            sphere_node_two->CreateComponent<ShadingComponent>(shader_);
            sphere_node_two->CreateComponent<RenderingComponent>(sphere_mesh_);
            sphere_node_two->CreateComponent<MaterialComponent>(std::make_shared<Material>(Material::GetDefault()));
            auto material_component_two = sphere_node_two->GetComponentPtr<MaterialComponent>();
            Material& material_two = material_component_two->GetMaterial();
            material_two.SetDiffuseColor(glm::vec3(0.0f, 1.0f, 0.0f));
            material_two.SetAmbientColor(glm::vec3(0.0f, 1.0f, 0.0f));
            sphere_nodes_.push_back(sphere_node_two.get());
            AddChild(std::move(sphere_node_two));

            auto sphere_node_three = make_unique<SceneNode>();
            sphere_node_three->GetTransform().SetPosition(glm::vec3(0, -1, 1));
            state_.positions.push_back(glm::vec3(0, -1, 1));
            state_.velocities.push_back(glm::vec3(0, 0, 0));
            system_.AddMass(0.005f);
            system_.AddSpring(1, 2, 0.5f, 0.5f);
            sphere_node_three->CreateComponent<ShadingComponent>(shader_);
            sphere_node_three->CreateComponent<RenderingComponent>(sphere_mesh_);
            sphere_node_three->CreateComponent<MaterialComponent>(std::make_shared<Material>(Material::GetDefault()));
            auto material_component_three = sphere_node_three->GetComponentPtr<MaterialComponent>();
            Material& material_three = material_component_three->GetMaterial();
            material_three.SetDiffuseColor(glm::vec3(0.0f, 1.0f, 0.0f));
            material_three.SetAmbientColor(glm::vec3(0.0f, 1.0f, 0.0f));
            sphere_nodes_.push_back(sphere_node_three.get());
            AddChild(std::move(sphere_node_three));

            auto sphere_node_four = make_unique<SceneNode>();
            sphere_node_four->GetTransform().SetPosition(glm::vec3(0, 2, 0));
            state_.positions.push_back(glm::vec3(0, 2, 0));
            state_.velocities.push_back(glm::vec3(0, 1, 0));
            system_.AddMass(0.005f);
            system_.AddSpring(2, 3, 0.5f, 0.5f);
            sphere_node_four->CreateComponent<ShadingComponent>(shader_);
            sphere_node_four->CreateComponent<RenderingComponent>(sphere_mesh_);
            sphere_node_four->CreateComponent<MaterialComponent>(std::make_shared<Material>(Material::GetDefault()));
            auto material_component_four = sphere_node_four->GetComponentPtr<MaterialComponent>();
            Material& material_four = material_component_four->GetMaterial();
            material_four.SetDiffuseColor(glm::vec3(0.0f, 1.0f, 0.0f));
            material_four.SetAmbientColor(glm::vec3(0.0f, 1.0f, 0.0f));
            sphere_nodes_.push_back(sphere_node_four.get());
            AddChild(std::move(sphere_node_four));

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
