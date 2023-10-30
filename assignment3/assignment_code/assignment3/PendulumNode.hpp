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
            sphere_nodes_.push_back(sphere_node_four.get());
            AddChild(std::move(sphere_node_four));

            system_.FixMass(0);

            type_ = type;
            integrator_ = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(type_);

            step_size_ = step_size;
            time_ = 0.0;
        }

        void Update(double delta_time) override {
            state_ = integrator_->Integrate(system_, state_, time_, step_size_);
            
            for (int i = 0; i < sphere_nodes_.size(); i++) {
                sphere_nodes_[i]->GetTransform().SetPosition(state_.positions[i]);
            }

            time_ += delta_time;
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
