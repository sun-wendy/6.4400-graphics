#include "Tracer.hpp"

#include <glm/gtx/string_cast.hpp>
#include <stdexcept>
#include <algorithm>

#include "gloo/Transform.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/lights/AmbientLight.hpp"

#include "gloo/Image.hpp"
#include "Illuminator.hpp"

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

      glm::vec3 color = glm::vec3(0.0f);
      Ray ray = camera_.GenerateRay(glm::vec2(x, y));
      HitRecord record;
      color = TraceRay(ray, max_bounces_, record);
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

  for (const auto& single_tracing : tracing_components_) {
    glm::mat4 transform = single_tracing->GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
    glm::mat4 inv = glm::inverse(transform);
    Ray temp_ray = ray;
    temp_ray.ApplyTransform(inv);
    HitRecord temp_record;
    
    const auto& hittable = single_tracing->GetHittable();
    if (hittable.Intersect(temp_ray, camera_.GetTMin(), temp_record)) {
      temp_ray.ApplyTransform(transform);

      if (temp_record.time < record.time) {
        record = temp_record;
        record.normal = glm::normalize(glm::transpose(glm::inverse(glm::mat3(transform))) * temp_record.normal);
      }
    }
  }
}


glm::vec3 Tracer::GetBackgroundColor(const glm::vec3& direction) const {
  if (cube_map_ != nullptr) {
    return cube_map_->GetTexel(direction);
  } else
    return background_color_;
}
}  // namespace GLOO
