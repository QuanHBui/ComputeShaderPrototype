/**
 * The goal is to implement this on the gpu, specifically the compute shader
 *
 */

#include "P3NarrowPhaseCollisionDetection.h"

#define EPSILON 0.000001f

// We are going old school
#define ISECT(projVert0, projVert1, projVert2, distVert0, distVert1, distVert2, isectStart, isectEnd)	\
			isectStart = projVert0 + (projVert1 - projVert0) * distVert0/(distVert0 - distVert1);		\
			isectEnd = projVert0 + (projVert2 - projVert0) * distVert0/(distVert0 - distVert2);

// Test for intersection between coplanar triangles
bool coplanarTriTriTest(glm::vec3 const &v0, glm::vec3 const &v1, glm::vec3 const &v2,
						glm::vec3 const &u0, glm::vec3 const &u1, glm::vec3 const &u2,
						glm::vec3 const &N1)
{
	return false;
}

// Fast test for general 3D tri tri intersection. Does not return intersection segment
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
	float prodDistU0DistU1 = distU0 * distU1;
	float prodDistU0DistU2 = distU0 * distU2;
	if (prodDistU0DistU1 > 0.0f && prodDistU0DistU2 > 0.0f)
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
	float prodDistV0DistV1 = distV0 * distV1;
	float prodDistV0DistV2 = distV0 * distV2;
	if (prodDistV0DistV1 > 0.0f && prodDistV0DistV2 > 0.0f)
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

	float isect0[2];
	float isect1[2];
	bool isCoplanar = false;

	// Compute intersection interval for triangle 1
	computeIntersectInterval(projV0, projV1, projV2, distV0, distV1, distV2,
							prodDistV0DistV1, prodDistV0DistV2,
							isect0[0], isect0[1], isCoplanar);

	// Compute intersection interval for triangle 2
	computeIntersectInterval(projU0, projU1, projU2, distU0, distU1, distU2,
							prodDistU0DistU1, prodDistU0DistU2,
							isect1[0], isect1[1], isCoplanar);

	// If the first triangle is coplanar, then the second should too, so
	//  perform this check only once.
	if (isCoplanar)
		return coplanarTriTriTest(v0, v1, v2, u0, u1, u2, N1);

	return true;
}

void computeIntersectInterval(	float projVert0, float projVert1, float projVert2,
								float distVert0, float distVert1, float distVert2,
								float prodDistVert0DistVert1, float prodDistVert0DistVert2,
								float &isectStart, float &isectEnd, bool &isCoplanar)
{
	// Check for which 2 edges are intersecting the plane by looking at the
	//  product of their vertices' signed distances.

	// Vert0 and Vert1 on the same side, look at edge Vert0...Vert2 and
	//  edge Vert1...Vert2
	if (prodDistVert0DistVert1 > 0.0f) {
		ISECT(projVert2, projVert0, projVert1, distVert2, distVert0, distVert1, isectStart, isectEnd);
	}
	// Vert0 and Vert2 on the same side, look at edge Vert0...Vert1 and
	//  edge Vert2...Vert1
	else if (prodDistVert0DistVert2 > 0.0f) {
		ISECT(projVert1, projVert0, projVert2, distVert1, distVert0, distVert2, isectStart, isectEnd);
	}
	// Vert1 and Vert2 on the same side, look at edge Vert1...Vert0 and
	//  edge Vert2...Vert0. Note that there's an extra check if Vert0 is
	//  in the plane.
	else if (distVert1*distVert2 > 0.0f || distVert0 != 0.0f) {
		ISECT(projVert0, projVert1, projVert2, distVert0, distVert1, distVert2, isectStart, isectEnd);
	}
	// At this point, Vert0 is in the plane.
	else if (distVert1 != 0.0f) {
		ISECT(projVert1, projVert0, projVert2, distVert1, distVert0, distVert2, isectStart, isectEnd);
	}
	// Both Vert0 and Vert1 are in the plane.
	else if (distVert2 != 0.0f) {
		ISECT(projVert2, projVert0, projVert1, distVert2, distVert0, distVert1, isectStart, isectEnd);
	}
	// Triangle is coplanar to the plane.
	else {
		isCoplanar = true;
	}
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