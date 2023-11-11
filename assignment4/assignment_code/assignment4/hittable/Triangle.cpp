#include "Triangle.hpp"

#include <iostream>
#include <stdexcept>

#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Plane.hpp"

namespace GLOO {
Triangle::Triangle(const glm::vec3& p0,
                   const glm::vec3& p1,
                   const glm::vec3& p2,
                   const glm::vec3& n0,
                   const glm::vec3& n1,
                   const glm::vec3& n2) {
  positions_.push_back(p0);
  positions_.push_back(p1);
  positions_.push_back(p2);
  normals_.push_back(n0);
  normals_.push_back(n1);
  normals_.push_back(n2);
}

Triangle::Triangle(const std::vector<glm::vec3>& positions,
                   const std::vector<glm::vec3>& normals) {
}

bool Triangle::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  // TODO: Implement ray-triangle intersection.
  
  glm::vec3 R_0 = ray.GetOrigin();
  glm::vec3 R_d = ray.GetDirection();

  glm::mat3 A = glm::mat3(positions_[0] - positions_[1], positions_[0] - positions_[2], R_d);
  glm::vec3 b = positions_[0] - R_0;
  glm::vec3 x = glm::inverse(A) * b;
  
  float beta = x[0];
  float gamma = x[1];
  float alpha = 1 - beta - gamma;
  float t = x[2];

  if (beta + gamma > 1 || beta < 0 || gamma < 0 || t < t_min) {
    return false;
  } else{
    if (t < record.time) {
      record.time = t;
      record.normal = glm::normalize(alpha * normals_[0] + beta * normals_[1] + gamma * normals_[2]);  // Interpolate normals
      return true;
    }
  }
  return false;
}
}  // namespace GLOO
