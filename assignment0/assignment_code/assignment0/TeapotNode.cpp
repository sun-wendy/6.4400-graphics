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
    // Constructor
    std::shared_ptr<PhongShader> shader = std::make_shared<PhongShader>();
    std::shared_ptr<VertexObject> mesh = MeshLoader::Import("assignment0/teapot.obj").vertex_obj;
    if (mesh == nullptr) {
        return;
    }
    // Don't need to create a new SceneNode - like self. in Python (office hours)
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
    std::list<glm::vec3> color_list = {glm::vec3(0.0f, 0.5f, 1.0f), glm::vec3(0.0f, 0.5f, 0.3f), glm::vec3(0.2f, 0.8f, 0.5f)};

    if (InputManager::GetInstance().IsKeyPressed('C')) {
        if (prev_released) {
            std::cout << "Toggling color" << std::endl;

            // Get the current culor (went to office hours for help on this)
            auto material_component = GetComponentPtr<MaterialComponent>();
            Material& material = material_component->GetMaterial();
            glm::vec3 cur_color = material.GetDiffuseColor();

            // Iterate through the color list
            std::list<glm::vec3>::iterator index = std::find(color_list.begin(), color_list.end(), cur_color);
            if (index == color_list.end()) {
                cur_color = color_list.front();
            } else {
                index++;
                if (index == color_list.end()) {
                    cur_color = color_list.front();
                } else {
                    cur_color = *index;
                }
            }

            // Set to new color
            material.SetDiffuseColor(cur_color);
            material.SetAmbientColor(cur_color);
        }
        prev_released = false;
    } else if (InputManager::GetInstance().IsKeyReleased('C')) {
        prev_released = true;
    }
}
}
