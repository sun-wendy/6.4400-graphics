#include "Plane.hpp"

namespace GLOO {
Plane::Plane(const glm::vec3& normal, float d) {
  normal_ = normal;
  d_ = -d;
}

bool Plane::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  // TODO: Implement ray-plane intersection.
  
  glm::vec3 R_0 = ray.GetOrigin();
  glm::vec3 R_d = ray.GetDirection();

  float t = -(d_ + glm::dot(normal_, R_0)) / glm::dot(normal_, R_d);

  if (t < t_min) {
    return false;
  } else{
    if (t < record.time) {
      record.time = t;
      record.normal = glm::normalize(normal_);
      return true;
    }
  }
  return false;

}
}  // namespace GLOO
