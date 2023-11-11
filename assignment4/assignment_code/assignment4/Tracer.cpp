#include "Tracer.hpp"

#include <glm/gtx/string_cast.hpp>
#include <stdexcept>
#include <algorithm>

#include "gloo/Transform.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/lights/AmbientLight.hpp"

#include "gloo/Image.hpp"
#include "Illuminator.hpp"

#include "glm/gtx/string_cast.hpp"

namespace GLOO {
void Tracer::Render(const Scene& scene, const std::string& output_file) {
  scene_ptr_ = &scene;

  auto& root = scene_ptr_->GetRootNode();
  tracing_components_ = root.GetComponentPtrsInChildren<TracingComponent>();
  light_components_ = root.GetComponentPtrsInChildren<LightComponent>();


  Image image(image_size_.x, image_size_.y);


  for (size_t y = 0; y < image_size_.y; y++) {
    for (size_t x = 0; x < image_size_.x; x++) {
      // TODO: For each pixel, cast a ray, and update its value in the image.
      
      float x_norm = float(x) / image_size_.x * 2 - 1;
      float y_norm = float(y) / image_size_.y * 2 - 1;
      Ray ray = camera_.GenerateRay(glm::vec2(x_norm, y_norm));
      HitRecord record;
      glm::vec3 color = TraceRay(ray, max_bounces_, record);
      image.SetPixel(x, y, color);
    }
  }

  if (output_file.size())
    image.SavePNG(output_file);
}


glm::vec3 Tracer::TraceRay(const Ray& ray,
                           size_t bounces,
                           HitRecord& record) const {
  // TODO: Compute the color for the cast ray.
  // return GetBackgroundColor(ray.GetDirection());

  glm::vec3 pixel_color(0.0f);
  bool hit = false;

  for (auto& single_tracing : tracing_components_) {
    glm::mat4 transform = single_tracing->GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
    glm::mat4 inv = glm::inverse(transform);
    Ray temp_ray = ray;
    temp_ray.ApplyTransform(inv);

    const auto& hittable = single_tracing->GetHittable();
    if (hittable.Intersect(temp_ray, camera_.GetTMin(), record)) {
      hit = true;

      record.normal = glm::normalize(glm::transpose(glm::inverse(glm::mat3(transform))) * record.normal);
      pixel_color = glm::vec3(0.0f);
      
      temp_ray.ApplyTransform(transform);
      const glm::vec3& hit_pos = temp_ray.At(record.time);

      glm::vec3 I(0.0f);
      glm::vec3 I_indirect(0.0f);

      for (auto& single_light : light_components_) {
        // Point light & directional light
        if (single_light->GetLightPtr()->GetType() == LightType::Point || single_light->GetLightPtr()->GetType() == LightType::Directional) {
          // Diffuse shading
          glm::vec3 dir_to_light(0.0f);
          glm::vec3 intensity(0.0f);
          float dist_to_light = 0.0f;
          Illuminator::GetIllumination(*single_light, hit_pos, dir_to_light, intensity, dist_to_light);
          glm::vec3 k_diffuse = single_tracing->GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial().GetDiffuseColor();
          glm::vec3 I_diffuse = GetDiffuseShading(dir_to_light, record.normal, intensity, k_diffuse);

          // Specular shading
          glm::vec3 surface_to_eye = temp_ray.GetDirection();
          glm::vec3 k_specular = single_tracing->GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial().GetSpecularColor();
          float shininess = single_tracing->GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial().GetShininess();
          glm::vec3 I_specular = GetSpecularShading(shininess, dir_to_light, surface_to_eye, record.normal, intensity, k_specular);
          
          I += (I_diffuse + I_specular);
        }

        // Ambient light
        if (single_light->GetLightPtr()->GetType() == LightType::Ambient) {
          glm::vec3 k_ambient = single_tracing->GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial().GetAmbientColor();
          glm::vec3 L_ambient = single_light->GetLightPtr()->GetDiffuseColor();
          glm::vec3 I_ambient = k_ambient * L_ambient;
          I += I_ambient;
        }
      }

      // Secondary rays
      if (bounces > 0) {
        HitRecord new_record;
        glm::vec3 R = ray.GetDirection() - 2 * glm::dot(ray.GetDirection(), record.normal) * record.normal;
        glm::vec3 R_epsilon = R * glm::vec3(0.01);
        Ray reflected(hit_pos + R_epsilon, R);
        I_indirect = (TraceRay(reflected, bounces - 1, new_record) * single_tracing->GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial().GetSpecularColor());
      }

      pixel_color = I + I_indirect;
    }
  }

  // if (record.time < std::numeric_limits<float>::max()) {
  //   return pixel_color;
  // } else {
  //   return GetBackgroundColor(ray.GetDirection());
  // }

  if (hit) {
    return pixel_color;
  } else {
    return GetBackgroundColor(ray.GetDirection());
  }
}


glm::vec3 Tracer::GetDiffuseShading(glm::vec3& light_dir,
                                    glm::vec3& normal,
                                    glm::vec3& intensity,
                                    glm::vec3& k_diffuse) const {
  
  float clamped = std::max(0.0f, glm::dot(light_dir, normal));
  glm::vec3 I_diffuse = clamped * intensity * k_diffuse;
  return I_diffuse;
}


glm::vec3 Tracer::GetSpecularShading(float shininess,
                                      glm::vec3& light_dir,
                                      glm::vec3& surface_to_eye,
                                      glm::vec3& normal,
                                      glm::vec3& intensity,
                                      glm::vec3& k_specular) const {
  
  glm::vec3 reflected_eye = surface_to_eye - 2 * glm::dot(surface_to_eye, normal) * normal;
  float clamped = std::max(0.0f, glm::dot(light_dir, reflected_eye));
  glm::vec3 I_specular = pow(clamped, shininess) * intensity * k_specular;
  return I_specular;
}


glm::vec3 Tracer::GetBackgroundColor(const glm::vec3& direction) const {
  if (cube_map_ != nullptr) {
    return cube_map_->GetTexel(direction);
  } else
    return background_color_;
}
}  // namespace GLOO
