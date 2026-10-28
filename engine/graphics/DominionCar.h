#ifndef DOMINION_CAR_H
#define DOMINION_CAR_H

#include <array>
#include "Mesh.h"

namespace RacingEngine {

/** RD-01: an original low-poly silhouette built in game units, facing +Z. */
class DominionCar {
public:
    void initialize();
    void setPosition(const Vector3& position);
    void setRotation(float angle, const Vector3& axis);
    void draw(const class Shader& shader, bool braking = false) const;
    void destroy();

private:
    std::array<Model, 5> parts;
};

} // namespace RacingEngine
#endif
