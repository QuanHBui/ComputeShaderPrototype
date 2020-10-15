#include "P3Collider.h"

#include <glm/glm.hpp>
#include <limits>

/**
 * Look at a certain direction and return the farthest point in that 
 */
glm::vec3 P3MeshCollider::findFarthestPoint(glm::vec3 const& direction) const
{
	glm::vec3 maxPoint{ 0.0f };
	float maxProjectedDistance = std::numeric_limits<float>::min();

	for (size_t i = 0u; i < mVertices.size() - 3u; i += 3u)
	{
		glm::vec3 vertex{ mVertices[i], mVertices[i + 1], mVertices[i + 2] };
		float projectedDistance = glm::dot(vertex, direction);
		
		if (projectedDistance > maxProjectedDistance)
		{
			maxProjectedDistance = projectedDistance;
			maxPoint = vertex;
		}
	}

	return maxPoint;
}