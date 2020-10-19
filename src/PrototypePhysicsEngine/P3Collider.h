#pragma once

#ifndef P3_COLLIDER_H
#define P3_COLLIDER_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

/**
 * @reference: https://blog.winter.dev/2020/gjk-algorithm/
 */

class P3Collider
{
public:
	virtual glm::vec3 findFarthestPoint(glm::vec3 const&) const = 0;
};

class P3MeshCollider : public P3Collider
{
public:
	void setModelMatrix(glm::mat4 const& modelMatrix)
	{
		mModelMatrix = modelMatrix;
	}
	
	void setVertices(std::vector<glm::vec4> const& vertices)
	{
		mVertices = vertices;
	}

	glm::vec3 findFarthestPoint(glm::vec3 const&) const override;

private:
	std::vector<glm::vec4> mVertices
	{
		glm::vec4{ -1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f,  1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f,  1.0f,  1.0f },

		glm::vec4{ -1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f, -1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f, -1.0f,  1.0f }
	};	// A unit box

	glm::mat4 mModelMatrix{ 1.0f };
};

/**
 * Take 2 Colliders and a direction, and determine the supporting point on the
 *  Minkowski difference.
 */
inline glm::vec3 computeSupportPoint(P3Collider const& pColliderA, P3Collider const& pColliderB,
	glm::vec3 const& direction)
{
	return pColliderA.findFarthestPoint(direction) - pColliderB.findFarthestPoint(-direction);
}

#endif // P3_COLLIDER_H