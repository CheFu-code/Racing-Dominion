#include "Vehicle.h"
#include <cmath>
#include <algorithm>

namespace RacingEngine {

Vehicle::Vehicle()
    : position(0, 0.5f, 0), velocity(0, 0, 0), acceleration(0, 0, 0),
      steeringAngle(0.0f), heading(0.0f), brakeInput(0.0f), speed(0.0f), maxSpeed(80.0f),
      accelerationForce(16000.0f), brakeForce(28000.0f),
      mass(1200.0f), drag(0.995f), friction(0.4f), turnSpeed(2.5f) {
}

void Vehicle::update(float deltaTime) {
    updatePhysics(deltaTime);
}

void Vehicle::applyAcceleration(float amount) {
    amount = fmaxf(0.0f, fminf(1.0f, amount));

    // Calculate forward direction based on current heading
    float forwardX = sinf(heading);
    float forwardZ = cosf(heading);

    float force = accelerationForce * amount / mass;
    acceleration.x += forwardX * force;
    acceleration.z += forwardZ * force;
}

void Vehicle::applyBrake(float amount) {
    brakeInput = fmaxf(0.0f, fminf(1.0f, amount));
}

void Vehicle::applySteering(float angle, float deltaTime) {
    steeringAngle = angle;

    // Apply steering force proportional to speed
    if (speed > 1.0f) {
        float steerForce = angle * turnSpeed * fminf(speed / 20.0f, 1.0f);

        // Don't apply lateral acceleration directly to velocity
        // Instead, rotate the velocity vector
        float rotAngle = steerForce * deltaTime;
        float cosA = cosf(rotAngle);
        float sinA = sinf(rotAngle);
        float newVx = velocity.x * cosA - velocity.z * sinA;
        float newVz = velocity.x * sinA + velocity.z * cosA;
        velocity.x = newVx;
        velocity.z = newVz;
        heading = atan2f(velocity.x, velocity.z);
    }
}

float Vehicle::getSpeed() const {
    return sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
}

void Vehicle::updatePhysics(float deltaTime) {
    // Update velocity
    velocity.x += acceleration.x * deltaTime;
    velocity.y += acceleration.y * deltaTime;
    velocity.z += acceleration.z * deltaTime;

    // Apply drag (air resistance)
    float frameDrag = powf(drag, deltaTime * 60.f);
    velocity.x *= frameDrag;
    velocity.z *= frameDrag;

    // Clamp the braking impulse to the remaining speed so braking cannot reverse the car.
    float beforeBrake = getSpeed();
    if (beforeBrake > 0 && brakeInput > 0) {
        float reduction = fminf(beforeBrake, brakeForce / mass * brakeInput * deltaTime);
        float factor = (beforeBrake - reduction) / beforeBrake;
        velocity.x *= factor;
        velocity.z *= factor;
    }
    brakeInput = 0;

    // Apply ground friction when not accelerating
    speed = getSpeed();
    if (speed > 0.01f && acceleration.lengthSquared() < 0.001f) {
        float frictionForce = friction * deltaTime;
        if (frictionForce > speed) frictionForce = speed;
        float factor = (speed - frictionForce) / speed;
        velocity.x *= factor;
        velocity.z *= factor;
    }

    // Clamp to max speed
    clampSpeed();

    // Update position
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
    position.z += velocity.z * deltaTime;

    // Keep car at ground level
    if (position.y < 0.5f) {
        position.y = 0.5f;
        velocity.y = 0;
    }

    // Reset acceleration
    acceleration = Vector3(0, 0, 0);

    // Update speed
    speed = getSpeed();
}

void Vehicle::clampSpeed() {
    float currentSpeed = getSpeed();
    if (currentSpeed > maxSpeed) {
        float scale = maxSpeed / currentSpeed;
        velocity.x *= scale;
        velocity.z *= scale;
    }
}

} // namespace RacingEngine
