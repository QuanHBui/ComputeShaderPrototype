#include "P3Collider.h"

#include <glm/glm.hpp>
#include <limits>

/**
 * Look at a certain direction and return the farthest point in that.
 *
 * TODO: This seems to be trivally parallelizable.
 */
glm::vec3 P3MeshCollider::findFarthestPoint(glm::vec3 const& direction) const
{
	glm::vec3 maxPoint{ 0.0f };
	float maxProjectedDistance = std::numeric_limits<float>::min();

	std::vector<glm::vec4>::const_iterator vertex;
	for (vertex = mVertices.begin(); vertex != mVertices.end(); ++vertex)
	{
		glm::vec3 vert = glm::vec3(mModelMatrix * *vertex);
		float projectedDistance = glm::dot(vert, direction);

		if (projectedDistance > maxProjectedDistance)
		{
			maxProjectedDistance = projectedDistance;
			maxPoint = vert;
		}
	}

	return maxPoint;
}