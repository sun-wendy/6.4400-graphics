#ifndef TRAPEZOIDAL_INTEGRATOR_H_
#define TRAPEZOIDAL_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class TrapezoidalIntegrator : public IntegratorBase<TSystem, TState> {
  TState Integrate(const TSystem& system,
                   const TState& state,
                   float start_time,
                   float dt) const override {

    ParticleState force_0 = system.ComputeTimeDerivative(state, start_time);
    ParticleState force_1 = system.ComputeTimeDerivative(state + dt * force_0, start_time + dt);
    TState state_new = state + 0.5f * dt * (force_0 + force_1);
    return state_new;
  }
};
}  // namespace GLOO

#endif
