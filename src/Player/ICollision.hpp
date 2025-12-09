#ifndef I_COLLISION_HPP
#define I_COLLISION_HPP

#include <glm/vec3.hpp>
#include "Helpers/AABB.hpp"

using glm::vec3;

class ICollision
{
public:
    virtual ~ICollision() = default;
    virtual vec3 resolve(const vec3& pos, const vec3& vel, const AABB& box) = 0;
    virtual bool isGrounded() const = 0;
};

#endif // I_COLLISION_HPP
