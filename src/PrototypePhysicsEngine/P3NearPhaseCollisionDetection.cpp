#include "P3NearPhaseCollisionDetection.h"

#define EPSILON 0.000001f

// Test for intersection between coplanar triangles
bool coplanarTriTriTest(glm::vec3 const &v0, glm::vec3 const &v1, glm::vec3 const &v2,
						glm::vec3 const &u0, glm::vec3 const &u1, glm::vec3 const &u2)
{

}

// Fast test for general 3D tri tri intersection
bool fastTriTriIntersect3DTest(	glm::vec3 const &v0, glm::vec3 const &v1, glm::vec3 const &v2,
							glm::vec3 const &u0, glm::vec3 const &u1, glm::vec3 const &u2)
{
	// 2 edges originating from v0 of the first triangle
	glm::vec3 p1 = v1 - v0;
	glm::vec3 p2 = v2 - v0;

	// Compute plane equation of first triangle
	glm::vec3 N1 = glm::cross(p1, p2);
	float d1 = -glm::dot(N1, v0);

	// Signed distances of u0, u1, and u2 to plane of first triangle
	float distU0 = glm::dot(N1, u0) + d1;
	float distU1 = glm::dot(N1, u1) + d1;
	float distU2 = glm::dot(N1, u2) + d1;

	// Check for coplanarity. Using epsilon check because float precision
	if (glm::abs(distU0) < EPSILON) distU0 = 0.0f;
	if (glm::abs(distU1) < EPSILON) distU1 = 0.0f;
	if (glm::abs(distU2) < EPSILON) distU2 = 0.0f;
	// If same sign and non-zero, no intersection. Early rejection
	if (distU0*distU1 > 0.0f && distU0*distU2 > 0.0f)
		return false;

	// 2 edges originating from u0 of the second triangle
	glm::vec3 q1 = u1 - u0;
	glm::vec3 q2 = u2 - u0;

	// Compute plane equation of second triangle
	glm::vec3 N2 = glm::cross(q1, q2);
	float d2 = -glm::dot(N2, u0);

	// Signed distances of v0, v1, and v2 to plane of second triangle
	float distV0 = glm::dot(N2, v0) + d2;
	float distV1 = glm::dot(N2, v1) + d2;
	float distV2 = glm::dot(N2, v2) + d2;

	// Check for coplanarity. Using epsilon check because float precision
	if (glm::abs(distV0) < EPSILON) distV0 = 0.0f;
	if (glm::abs(distV1) < EPSILON) distV1 = 0.0f;
	if (glm::abs(distV2) < EPSILON) distV2 = 0.0f;
	// If same sign and non-zero, no intersection. Early rejection
	if (distV0*distV1 > 0.0f && distV0*distV2 > 0.0f)
		return false;

	// Compute direction of intersection line
	glm::vec3 intersectLineDirection = glm::cross(N1, N2);

	// Optimization. Determine which component of intersectLineDirection is the max
	float maxComp = glm::abs(intersectLineDirection.x);
	int index = 0u;
	float yComp = glm::abs(intersectLineDirection.y);
	float zComp = glm::abs(intersectLineDirection.z);
	if (yComp > maxComp) maxComp = yComp, index = 1u;
	if (zComp > maxComp) maxComp = zComp, index = 2u;

	// Simplified projection of each vertex from triangle 1 and 2 onto intersectionLine L
	float projV0 = v0[index];
	float projV1 = v1[index];
	float projV2 = v2[index];

	float projU0 = u0[index];
	float projU1 = u1[index];
	float projU2 = u2[index];

	// Compute intersection interval for triangle 1

	// Compute intersection interval for triangle 2

	return true;
}

// Edge to edge test
// Reference: Franlin Antonio's "Faster Line Segment Intersection"
//				in Graphics Gem III pg 199-202
bool edgeEdgeTest(glm::vec3 const &v0, glm::vec3 const &u0, glm::vec3 const &u1)
{
	return false;
}

bool edgeTriTest(	glm::vec3 const &v0, glm::vec3 const &v1,
					glm::vec3 const &u0, glm::vec3 const &u1, glm::vec3 const &u2)
{
	return false;
}

bool pointInTriTest(glm::vec3 const &v0,
					glm::vec3 const &u0, glm::vec3 const &u1, glm::vec3 const &u2)
{
	return false;
}