#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP

#include <glm/glm.hpp>

using glm::vec3;

struct Frustum
{
    static constexpr auto MARGIN = 0.6f;
    static constexpr auto EXTRA_RENDER_DISTANCE = 128.0f;
    static constexpr auto MIN_RENDER_DISTANCE = 80.0f;
    static constexpr auto CHUNK_RADIUS = 70.0f;

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
        
        if (dist > max_distance + EXTRA_RENDER_DISTANCE) return false;

        if (dist < MIN_RENDER_DISTANCE) return true;

        vec3 to_chunk = glm::normalize(center - cam_pos);

        float angle_cos = glm::dot(cam_front, to_chunk);

        float angular_radius = atan2(CHUNK_RADIUS, dist);
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
