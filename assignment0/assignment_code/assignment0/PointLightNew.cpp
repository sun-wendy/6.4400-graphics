#include "PointLightNew.hpp"

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
#include "gloo/external.hpp"

namespace GLOO {
PointLightNew::PointLightNew() {
    // Constructor
    auto point_light = std::make_shared<PointLight>();
    point_light->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
    point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
    point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
    CreateComponent<LightComponent>(point_light);
    GetTransform().SetPosition(glm::vec3(0.0f, 4.0f, 5.f));
}

void PointLightNew::Update(double delta_time) {
    // Update
    static bool prev_released_up = true;
    static bool prev_released_down = true;
    static bool prev_released_left = true;
    static bool prev_released_right = true;

    ImGuiIO& io = ImGui::GetIO();

    // Moving point light up
    if (io.KeysDown[GLFW_KEY_UP]) {
        if (prev_released_up) {
            std::cout << "Moving point light UP" << std::endl;
            // Get the current light position & move up a little
            glm::vec3 cur_pos = GetTransform().GetPosition();
            cur_pos.y += 0.2f;
            GetTransform().SetPosition(cur_pos);
        }
        prev_released_up = false;
    } else if (!io.KeysDown[GLFW_KEY_UP]) {
        prev_released_up = true;
    }

    // Moving point light down
    if (io.KeysDown[GLFW_KEY_DOWN]) {
        if (prev_released_down) {
            std::cout << "Moving point light DOWN" << std::endl;
            // Get the current light position & move down a little
            glm::vec3 cur_pos = GetTransform().GetPosition();
            cur_pos.y -= 0.2f;
            GetTransform().SetPosition(cur_pos);
        }
        prev_released_down = false;
    } else if (!io.KeysDown[GLFW_KEY_DOWN]) {
        prev_released_down = true;
    }

    // Moving point light left
    if (io.KeysDown[GLFW_KEY_LEFT]) {
        if (prev_released_left) {
            std::cout << "Moving point light LEFT" << std::endl;
            // Get the current light position & move left a little
            glm::vec3 cur_pos = GetTransform().GetPosition();
            cur_pos.x -= 0.2f;
            GetTransform().SetPosition(cur_pos);
        }
        prev_released_left = false;
    } else if (!io.KeysDown[GLFW_KEY_LEFT]) {
        prev_released_left = true;
    }

    // Moving point light right
    if (io.KeysDown[GLFW_KEY_RIGHT]) {
        if (prev_released_right) {
            std::cout << "Moving point light RIGHT" << std::endl;
            // Get the current light position & move right a little
            glm::vec3 cur_pos = GetTransform().GetPosition();
            cur_pos.x += 0.2f;
            GetTransform().SetPosition(cur_pos);
        }
        prev_released_right = false;
    } else if (!io.KeysDown[GLFW_KEY_RIGHT]) {
        prev_released_right = true;
    }
}
}
