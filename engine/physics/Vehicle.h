#ifndef VEHICLE_H
#define VEHICLE_H

#include "Vector3.h"

namespace RacingEngine {

class Vehicle {
public:
    Vehicle();

    void update(float deltaTime);
    void applyAcceleration(float amount);
    void applyBrake(float amount);
    void applySteering(float angle, float deltaTime);

    void setPosition(const Vector3& pos) { position = pos; }
    Vector3 getPosition() const { return position; }
    Vector3 getVelocity() const { return velocity; }
    float getSpeed() const;
    float getSteeringAngle() const { return steeringAngle; }
    void setHeading(float radians) { heading = radians; }
    float getHeading() const { return heading; }

private:
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;

    float steeringAngle;
    float heading;
    float brakeInput;
    float speed;
    float maxSpeed;
    float accelerationForce;
    float brakeForce;
    float mass;
    float drag;
    float friction;
    float turnSpeed;

    void updatePhysics(float deltaTime);
    void clampSpeed();
};

} // namespace RacingEngine

#endif // VEHICLE_H
