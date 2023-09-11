#include "TeapotNode.hpp"

// More includes here
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/cameras/BasicCameraNode.hpp"
#include "gloo/InputManager.hpp"
#include <list>

namespace GLOO {
TeapotNode::TeapotNode() {
    int i = 0;

    std::shared_ptr<PhongShader> shader = std::make_shared<PhongShader>();
    std::shared_ptr<VertexObject> mesh = MeshLoader::Import("assignment0/teapot.obj").vertex_obj;
    if (mesh == nullptr) {
        return;
    }
    // auto replace_with_teapot = make_unique<SceneNode>();
    CreateComponent<ShadingComponent>(shader);
    CreateComponent<RenderingComponent>(mesh);

    // Create a MaterialComponent
    CreateComponent<MaterialComponent>(
        std::make_shared<Material>(Material::GetDefault()));
}

void TeapotNode::Update(double delta_time) {
    // User might press down the key across multiple frames
    static bool prev_released = true;
    // List of colors
    std::list<glm::vec3> color_list = {glm::vec3(0.0f, 0.5f, 0.2f), glm::vec3(0.0f, 0.5f, 0.3f), glm::vec3(0.0f, 0.5f, 0.4f)};

    if (InputManager::GetInstance().IsKeyPressed('C')) {
        if (prev_released) {
            std::cout << "Toggling color" << std::endl;
            // Toggle diffuse & ambient colors
            auto material_component = GetComponentPtr<MaterialComponent>();
            Material& material = material_component->GetMaterial();
            material.SetDiffuseColor(glm::vec3(0.0f, 0.5f, 1.0f));
            material.SetAmbientColor(glm::vec3(0.0f, 0.5f, 1.0f));
            // i++;
        }
        prev_released = false;
    } else if (InputManager::GetInstance().IsKeyReleased('C')) {
        prev_released = true;
    }
}
}
