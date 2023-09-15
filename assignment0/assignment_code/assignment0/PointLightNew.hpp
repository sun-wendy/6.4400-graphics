#ifndef POINT_LIGHT_NEW_H_
#define POINT_LIGHT_NEW_H_

#include "gloo/SceneNode.hpp"

namespace GLOO {
class PointLightNew : public SceneNode {
    public:
        PointLightNew();
        void Update(double delta_time) override;
};
}

#endif
