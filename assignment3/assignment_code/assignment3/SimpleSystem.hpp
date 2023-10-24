#ifndef SIMPLE_SYSTEM_H_
#define SIMPLE_SYSTEM_H_

#include "ParticleState.hpp"
#include "ParticleSystemBase.hpp"

namespace GLOO {
class SimpleSystem : public ParticleSystemBase {
   public:
      SimpleSystem() {}
      
      ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const {
         ParticleState derivative = ParticleState();

         for (int i = 0; i < state.positions.size(); i++) {
            derivative.positions.push_back(glm::vec3(-state.positions[i][1], state.positions[i][0], 0));
            derivative.velocities.push_back(glm::vec3(0, 0, 0));
         }

         return derivative;
   }
};
}  // namespace GLOO

#endif
