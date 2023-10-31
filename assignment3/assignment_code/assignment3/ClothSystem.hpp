#ifndef CLOTH_SYSTEM_H_
#define CLOTH_SYSTEM_H_

#include "ParticleState.hpp"
#include "ParticleSystemBase.hpp"

namespace GLOO {
class ClothSystem : public ParticleSystemBase {
   public:
      ClothSystem() {}
      
      ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const {
         std::vector<glm::vec3> velocities = state.velocities;
         std::vector<glm::vec3> accelerations;

         std::vector<glm::vec3> gravity;
         std::vector<glm::vec3> drag;
         std::vector<glm::vec3> spring_forces;

         for (int i = 0; i < velocities.size(); i++) {
            if (fixed_[i] == 1 || masses_[i] == 0.0f) {
               velocities[i] = glm::vec3(0.0f);
            }
            gravity.push_back(glm::vec3(0.0f, -9.8f, 0.0f) * masses_[i]);
            drag.push_back(-drag_cons_ * velocities[i]);
            spring_forces.push_back(glm::vec3(0.0f));

            for (int j = 0; j < velocities.size(); j++) {
               if (i != j) {
                  glm::vec3 distance = state.positions[i] - state.positions[j];
                  glm::vec3 cur_spring_force = -spring_cons_[i][j] * (glm::length(distance) - rest_leng_[i][j]) * glm::normalize(distance);
                  spring_forces[i] += cur_spring_force;
               }
            }
         }

         for (int i = 0; i < velocities.size(); i++) {
            if (fixed_[i] == 1 || masses_[i] == 0.0f) {
               accelerations.push_back(glm::vec3(0.0f));
            } else{
               if (!wind_on_) {
                  accelerations.push_back((gravity[i] + drag[i] + spring_forces[i]) / masses_[i]);
               } else {
                  float rand_num = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 0.1f);
                  glm::vec3 wind = glm::vec3(rand_num, 0.0f, 0.0f);
                  accelerations.push_back((gravity[i] + drag[i] + spring_forces[i] + wind) / masses_[i]);
               }
            }
         }
         
         ParticleState derivative;
         derivative.positions = velocities;
         derivative.velocities = accelerations;
         return derivative;
      }

      void AddMass(float mass) {
         masses_.push_back(mass);
         fixed_.push_back(0);

         if (masses_.size() == 0) {
            std::vector<float> init_row_rest_leng;
            std::vector<float> init_row_spring_cons;
            init_row_rest_leng.push_back(0.0f);
            init_row_spring_cons.push_back(0.0f);
            rest_leng_.push_back(init_row_rest_leng);
            spring_cons_.push_back(init_row_spring_cons);
         } else {
            std::vector<float> new_row_rest_leng;
            std::vector<float> new_row_spring_cons;
            for (int i = 0; i < rest_leng_.size(); i++) {
               rest_leng_[i].push_back(0.0f);
               new_row_rest_leng.push_back(0.0f);
               spring_cons_[i].push_back(0.0f);
               new_row_spring_cons.push_back(0.0f);
            }
            new_row_rest_leng.push_back(0.0f);
            new_row_spring_cons.push_back(0.0f);
            rest_leng_.push_back(new_row_rest_leng);
            spring_cons_.push_back(new_row_spring_cons);
         }
      }

      void AddSpring(int node_i, int node_j, float rest_leng, float spring_cons) {
         rest_leng_[node_i][node_j] = rest_leng;
         rest_leng_[node_j][node_i] = rest_leng;
         spring_cons_[node_i][node_j] = spring_cons;
         spring_cons_[node_j][node_i] = spring_cons;
      }

      void FixMass(int node_i) {
         fixed_[node_i] = 1;
      }

      int IndexOf(int i, int j) {
         return i * sqrt(masses_.size()) + j;
      }

      void AddRandomWind() {
         wind_on_ = true;
      }

      void RemoveWind() {
         wind_on_ = false;
      }

      bool IsWindOn() {
         return wind_on_;
      }

   private:
      std::vector<float> masses_;
      std::vector<int> fixed_;
      std::vector<std::vector<float>> rest_leng_;
      std::vector<std::vector<float>> spring_cons_;
      float drag_cons_ = 0.01f;
      bool wind_on_ = false;
};
}  // namespace GLOO

#endif
