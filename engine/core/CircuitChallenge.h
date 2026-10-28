#ifndef CIRCUIT_CHALLENGE_H
#define CIRCUIT_CHALLENGE_H

#include <array>
#include <cmath>

namespace RacingEngine {

// Pure race rules, shared by the engine and host tests. Medal values are 0..3.
class CircuitChallenge {
public:
    static constexpr int SECTORS = 12;
    std::array<float, 4> goldTargets{{8, 8, 8, 8}};
    std::array<int, SECTORS> medals{};
    float raceTime = 0, lapTime = 0, bestLap = 0, lastSectorTime = 0;
    int completed = 0, lastGate = 0, lastMedal = 0;

    void reset() {
        raceTime = lapTime = bestLap = lastSectorTime = sectorStart = 0;
        completed = lastGate = lastMedal = 0;
        medals.fill(0);
    }

    void tick(float dt) {
        if (!finished() && std::isfinite(dt) && dt > 0) { raceTime += dt; lapTime += dt; }
    }

    float target(int medal) const {
        return goldTargets[completed % 4] + (medal == 3 ? 0.f : medal == 2 ? 2.5f : 6.f);
    }

    static int rank(float seconds, float gold) {
        if (!std::isfinite(seconds) || seconds <= 0) return 0;
        return seconds <= gold ? 3 : seconds <= gold + 2.5f ? 2 : seconds <= gold + 6.f ? 1 : 0;
    }

    bool cross(int gate) {
        if (finished() || gate != (lastGate + 1) % 4 || raceTime <= sectorStart) return false;
        lastSectorTime = raceTime - sectorStart;
        lastMedal = rank(lastSectorTime, goldTargets[completed % 4]);
        medals[completed++] = lastMedal;
        lastGate = gate;
        sectorStart = raceTime;
        if (gate == 0) {
            if (bestLap == 0 || lapTime < bestLap) bestLap = lapTime;
            lapTime = 0;
        }
        return true;
    }

    bool finished() const { return completed == SECTORS; }
    float sectorTime() const { return raceTime - sectorStart; }
    int points() const { int sum = 0; for (int m : medals) sum += m; return sum; }
    int claimed() const { int count = 0; for (int m : medals) if (m) ++count; return count; }
    int dominionRank() const {
        if (!finished()) return 0;
        int score = points();
        return score >= 30 ? 3 : score >= 20 ? 2 : score >= 10 ? 1 : 0;
    }

private:
    float sectorStart = 0;
};
} // namespace RacingEngine
#endif
