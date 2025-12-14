#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP

#include <glm/glm.hpp>

using glm::vec3;

#define MARGIN 0.6f
#define EPSILON 32.0f

struct Frustum
{
    // radar
    void update(vec3 position, vec3 front, float fov, float range)
    {
        cam_pos = position;
        cam_front = glm::normalize(front);
        max_distance = range;

        halfFOV = glm::radians(fov / 2.0f);
        cutoff = cos(halfFOV + MARGIN);
    }

    bool isBoxVisible(const vec3& min, const vec3& max) const
    {
        vec3 center = (min + max) * 0.5f;

        float dist = glm::distance(cam_pos, center);
        
        if (dist > max_distance + 128.0f) return false;

        if (dist < 80.0f) return true;

        vec3 to_chunk = glm::normalize(center - cam_pos);

        float angle_cos = glm::dot(cam_front, to_chunk);

        float angular_radius = atan2(70.0f, dist);
        float local_cutoff = cos(halfFOV + angular_radius);

        if (angle_cos < local_cutoff)
        {
            return false;
        }

        return true;
    }

private:
    vec3 cam_pos;
    vec3 cam_front;
    float max_distance;
    float cutoff;
    float halfFOV;
};

#endif // FRUSTUM_HPP
