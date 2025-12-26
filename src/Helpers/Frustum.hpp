#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "World/Chunk.hpp"

struct Plane
{
	glm::vec3 normal;
	float distance;

	float getDistance(const glm::vec3& p) const
	{
		return glm::dot(normal, p) + distance;
	}
};

struct Frustum
{
private:
	Plane planes[6];

	static Plane normalizePlane(const glm::vec4& p)
	{
		glm::vec3 normal(p.x, p.y, p.z);
		float length = glm::length(normal);
		return { normal / length, p.w / length };
	}

public:
	Frustum(const glm::mat4& viewProj)
	{
		updateMatrix(viewProj);
	}

	void updateMatrix(const glm::mat4& vp)
	{
		// Left Plane
		planes[0] = normalizePlane(glm::vec4(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0], vp[3][3] + vp[3][0]));
		// Right Plane
		planes[1] = normalizePlane(glm::vec4(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0], vp[3][3] - vp[3][0]));
		// Bottom Plane
		planes[2] = normalizePlane(glm::vec4(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1], vp[3][3] + vp[3][1]));
		// Top Plane
		planes[3] = normalizePlane(glm::vec4(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1], vp[3][3] - vp[3][1]));
		// Near Plane
		planes[4] = normalizePlane(glm::vec4(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2], vp[3][3] + vp[3][2]));
		// Far Plane
		planes[5] = normalizePlane(glm::vec4(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2], vp[3][3] - vp[3][2]));

	}

	bool intersectsChunk(const Chunk& chunk) const
	{

		AABB box = chunk.getAABB();

		for (const auto& plane : planes)
		{
			glm::vec3 p = box.min;

			if (plane.normal.x >= 0) p.x = box.max.x;
			if (plane.normal.y >= 0) p.y = box.max.y;
			if (plane.normal.z >= 0) p.z = box.max.z;

			if (plane.getDistance(p) < 0)
			{
				return false;
			}
		}
		return true;
	}
};

#endif // FRUSTUM_HPP
