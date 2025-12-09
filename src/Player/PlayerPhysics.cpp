#include "Player/PlayerPhysics.hpp"

PlayerPhysics::PlayerPhysics(ICollision& collision_system_)
    : collision_system(collision_system_), position{0.f}, velocity {0.f}, 
      boundingBox{{-0.3f, 0.f, -0.3f}, {0.3f, 1.8f, 0.3f } }
{
}

void PlayerPhysics::jump(float force)
{
    if(collision_system.isGrounded())
        velocity.y = force;
}

void PlayerPhysics::move(const vec3& direction, float acceleration)
{
    velocity.x += direction.x * acceleration;
    velocity.z += direction.z * acceleration;
}

void PlayerPhysics::update(float dt)
{
    velocity.y += GRAVITY * dt;
    if (velocity.y < -MAX_FALL_SPEED) velocity.y = -MAX_FALL_SPEED;

    float current_drag = collision_system.isGrounded() ? DRAG : AIR_DRAG;

    // simple drag for x/z
    velocity.x *= std::max(0.f, 1.f - current_drag * dt);
    velocity.z *= std::max(0.f, 1.f - current_drag * dt);

    vec3 displacement = velocity * dt;

    position = collision_system.resolve(position, displacement, boundingBox);

    if(collision_system.isGrounded() && velocity.y < 0) velocity.y = 0;
}
