#ifndef TRACK_H
#define TRACK_H

#include <vector>
#include <string>
#include "Vector3.h"
#include "Mesh.h"

namespace RacingEngine {

struct TrackSegment {
    Vector3 center;
    float width;
    float length;
    float rotation; // Y-axis rotation in radians
};

struct BoundingBox {
    Vector3 min;
    Vector3 max;
};

class Track {
public:
    Track();
    ~Track();

    void generateDefaultTrack();
    void addSegment(const TrackSegment& segment);
    void buildMesh();

    void draw(const class Shader& shader) const;
    void destroy();

    bool checkCollision(const Vector3& position, float radius) const;
    bool isOnTrack(const Vector3& position) const;
    int getCheckpointIndex(const Vector3& position) const;

    const std::vector<TrackSegment>& getSegments() const { return segments; }
    Vector3 getStartPosition() const;
    float getStartRotation() const;
    int getTotalLaps() const { return totalLaps; }

private:
    std::vector<TrackSegment> segments;
    Mesh* trackMesh;
    Mesh* barrierMesh;
    Mesh* checkpointMesh;
    Mesh* groundMesh;
    Mesh* skylineMesh;
    int totalLaps;

    void createTrackGeometry();
    void createBarriers();
};

} // namespace RacingEngine

#endif // TRACK_H
