#ifndef SIMPLE_NODE_H_
#define SIMPLE_NODE_H_

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"

#include "ParticleState.hpp"
#include "IntegratorFactory.hpp"
#include "SimpleSystem.hpp"

namespace GLOO {
class SimpleNode : public SceneNode {
    public:
        SimpleNode(IntegratorType type, float step_size) {
            std::shared_ptr<VertexObject> sphere_mesh = PrimitiveFactory::CreateSphere(0.1f, 20, 20);
            std::shared_ptr<ShaderProgram> shader = std::make_shared<PhongShader>();

            auto sphere_node = make_unique<SceneNode>();
            sphere_node->GetTransform().SetPosition(glm::vec3(0, 0, 0));
            sphere_node->CreateComponent<ShadingComponent>(shader);
            sphere_node->CreateComponent<RenderingComponent>(sphere_mesh);

            sphere_node_ = sphere_node.get();
            AddChild(std::move(sphere_node));

            state_ = ParticleState();
            state_.positions.push_back(glm::vec3(0, 1, 1));
            state_.velocities.push_back(glm::vec3(0, 0, 0));
            system_ = SimpleSystem();

            type_ = type;
            if (type_ == IntegratorType::Euler) {
                integrator_ = IntegratorFactory::CreateIntegrator<SimpleSystem, ParticleState>(IntegratorType::Euler);
            } else {
                integrator_ = IntegratorFactory::CreateIntegrator<SimpleSystem, ParticleState>(IntegratorType::Trapezoidal);
            }

            step_size_ = step_size;
            time_ = 0.0;
        }

        void Update(double delta_time) override {
            state_ = integrator_->Integrate(system_, state_, time_, step_size_);
            sphere_node_->GetTransform().SetPosition(state_.positions[0]);
            time_ += delta_time;
        }

    private:
        SceneNode* sphere_node_;
        ParticleState state_;
        std::unique_ptr<IntegratorBase<SimpleSystem, ParticleState>> integrator_;
        SimpleSystem system_;
        IntegratorType type_;
        float step_size_;
        float time_;
};
}  // namespace GLOO

#endif