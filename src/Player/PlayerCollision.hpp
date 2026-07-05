#pragma once

#include "Player/ICollision.hpp"

class World;

class PlayerCollision : public ICollision
{
public:
    explicit PlayerCollision(World& world);

    vec3 resolve(const vec3& pos, const vec3& vel, const AABB& box) override;
    bool isGrounded() const override { return grounded; }

private:
    bool checkCollision(const AABB& box, const vec3& pos);
    
    World& world;
    bool grounded;
};
