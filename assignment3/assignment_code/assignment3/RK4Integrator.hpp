#ifndef RK4_INTEGRATOR_H_
#define RK4_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class RK4Integrator : public IntegratorBase<TSystem, TState> {
  TState Integrate(const TSystem& system,
                   const TState& state,
                   float start_time,
                   float dt) const override {
    
    TState k1 = system.ComputeTimeDerivative(state, start_time);
    TState k2 = system.ComputeTimeDerivative(state + 0.5f * dt * k1, 0.5f * start_time + dt);
    TState k3 = system.ComputeTimeDerivative(state + 0.5f * dt * k2, 0.5f * start_time + dt);
    TState k4 = system.ComputeTimeDerivative(state + dt * k3, start_time + dt);
    TState state_new = state + dt * (1 / 6.0f) * (k1 + 2.0f * k2 + 2.0f * k3 + k4);
    return state_new;
  }
};
}  // namespace GLOO

#endif
