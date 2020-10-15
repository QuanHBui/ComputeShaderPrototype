#pragma once

#ifndef P3_COLLIDER_H
#define P3_COLLIDER_H

#include <glm/vec3.hpp>
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
	glm::vec3 findFarthestPoint(glm::vec3 const&) const override;

private:
	std::vector<float> mVertices;
};

/**
 * Take 2 Colliders and a direction, and determine the supporting point on the
 *  Minkowski difference.
 */
inline glm::vec3 computeSupportPoint(P3Collider const* pColliderA, P3Collider const* pColliderB,
	glm::vec3 const& direction)
{

	return pColliderA->findFarthestPoint(direction) - pColliderB->findFarthestPoint(-direction);
}

#endif // P3_COLLIDER_H