#ifndef SMOKE_TRAIL_H
#define SMOKE_TRAIL_H

#include <array>
#include <cmath>
#include "Vector3.h"

namespace RacingEngine {
struct SmokePuff { Vector3 position; float age = 2, size = 0; };

// Bounded, deterministic particles; no allocation or random-number generation per frame.
class SmokeTrail {
public:
    std::array<SmokePuff, 32> puffs{};
    void reset() { for (auto& p : puffs) p.age = 2; clock = 0; cursor = 0; }
    void update(const Vector3& position, float heading, float amount, float dt) {
        for (auto& p : puffs) {
            p.age += dt;
            if (p.age < 1.1f) { p.position.y += dt * 0.6f; p.size += dt * 0.6f; }
        }
        clock += dt;
        if (amount < 0.1f || clock < 0.07f) return;
        clock = 0;
        Vector3 back(-sinf(heading), 0, -cosf(heading));
        Vector3 side(cosf(heading), 0, -sinf(heading));
        for (int sign : {-1, 1}) {
            puffs[cursor] = {position + back * 1.3f + side * (sign * 0.9f), 0, 0.35f};
            puffs[cursor].position.y = 0.3f;
            cursor = (cursor + 1) % puffs.size();
        }
    }
private:
    float clock = 0;
    std::size_t cursor = 0;
};
} // namespace RacingEngine
#endif
