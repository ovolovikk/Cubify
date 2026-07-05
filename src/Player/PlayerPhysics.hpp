#pragma once

#include "Player/ICollision.hpp"

class PlayerPhysics
{
public:
    explicit PlayerPhysics(ICollision& collision_system_);

    void update(float dt);

    bool jump(float force);
    void move(const vec3& direction, float acceleration);

    vec3 getPosition() const { return position; }
    void setPosition(const vec3& pos ) { position = pos; velocity = vec3(0.f); }
    vec3 getVelocity() const { return velocity; }
    bool isGrounded() const { return collision_system.isGrounded(); }

private:
    ICollision& collision_system;

    vec3 position;
    vec3 velocity;

    static constexpr auto MAX_FALL_SPEED = 50.0f;

    AABB boundingBox;
};
