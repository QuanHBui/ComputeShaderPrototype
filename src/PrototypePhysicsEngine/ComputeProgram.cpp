#include "ComputeProgram.h"

#include <iostream>
#include <stdexcept>

#include "GLSL.h"
#include "OpenGLUtils.h"

// When you don't care about elegance anymore
namespace shadersource
{
	// These are called raw string literals.
	constexpr char updateAabbs[] = R"updateAabbs(
#version 430 core

#define NUM_COLLIDER_VERTS 8u
#define MAX_NUM_COLLIDERS 1024u

// source: https://www.khronos.org/opengl/wiki/Small_Float_Formats
#define NEG_INF_32_BIT_FLOAT -3.4028237e35
#define POS_INF_32_BIT_FLOAT 3.4028237e35

layout(local_size_x = MAX_NUM_COLLIDERS) in;

struct BoxCollider
{
	vec4 vertices[NUM_COLLIDER_VERTS];
};

layout(std430, binding = 0) readonly buffer in_data
{
	ivec4 colliderMisc;
	BoxCollider boxColliders[];
};

layout(std430, binding = 1) writeonly buffer out_data
{
	vec4 minCoords[MAX_NUM_COLLIDERS];
	vec4 maxCoords[MAX_NUM_COLLIDERS];
};

uniform uint currNumColliders; a

void main()
{
	uint colliderIdx = gl_LocalInvocationID.x;

	if (colliderIdx >= currNumColliders)
	{
		minCoords[colliderIdx] = vec4(-1.0f, -1.0f, -1.0f, -2.0f);
		maxCoords[colliderIdx] = vec4(-1.0f, -1.0f, -1.0f, -2.0f);

		barrier();

		return;
	}

	BoxCollider boxCollider = boxColliders[colliderIdx];

	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = POS_INF_32_BIT_FLOAT;
	maxX = maxY = maxZ = NEG_INF_32_BIT_FLOAT;

	vec3 vertex = vec3(0.0f, 0.0f, 0.0f);

	// Go through all vertices
	for (int i = 0; i < NUM_COLLIDER_VERTS; ++i)
	{
		// Go through each dimension
		vertex = boxCollider.vertices[i].xyz;

		minX = min(minX, vertex.x);
		maxX = max(maxX, vertex.x);

		minY = min(minY, vertex.y);
		maxY = max(maxY, vertex.y);

		minZ = min(minZ, vertex.z);
		maxZ = max(maxZ, vertex.z);
	}

	// Store the results
	minCoords[colliderIdx] = vec4(minX, minY, minZ, colliderIdx); // Store the entity unique ID in the w comp
	maxCoords[colliderIdx] = vec4(maxX, maxY, maxZ, colliderIdx);

	barrier();
}
	)updateAabbs";

//===========================================================================//

	constexpr char sap[] = R"sap(
#version 430

#define MAX_NUM_COLLIDERS 1024

precision highp float;

layout(local_size_x = MAX_NUM_COLLIDERS) in;

layout(std430, binding = 1) volatile buffer in_sorted_data
{
	vec4 minCoords[MAX_NUM_COLLIDERS];
	vec4 maxCoords[MAX_NUM_COLLIDERS];
};

layout(std430, binding = 2) volatile buffer out_collision_pairs_data
{
	ivec4 collisionPairMisc;
	ivec4 collisionPairs[2 * MAX_NUM_COLLIDERS];
};

layout(binding = 0) uniform atomic_uint nextAvailableIdx;

shared ivec2 localCollisionPairs[2 * MAX_NUM_COLLIDERS];

uniform uint currNumColliders;

subroutine void sweep(uint i, uint j);

subroutine uniform sweep sweepAxis;

subroutine(sweep)
void sweepX(uint i, uint j)
{
	// Check for collision on the x-axis
	if (minCoords[i].x < maxCoords[j].x && maxCoords[i].x > minCoords[j].x)
	{
		uint availableIdx = atomicCounterIncrement(nextAvailableIdx);
		collisionPairs[availableIdx].x = int(minCoords[i].w); // w comp stores object ID
		collisionPairs[availableIdx].y = int(minCoords[j].w);
	}
}

subroutine(sweep)
void sweepY(uint i, uint j)
{
	// Look through the previous collision pair buffer
	for (int collisionPairIdx = 0; collisionPairIdx < 2 * currNumColliders; ++collisionPairIdx)
	{
		if (((int(minCoords[i].w) == localCollisionPairs[collisionPairIdx].x && int(minCoords[j].w) == localCollisionPairs[collisionPairIdx].y) ||
			 (int(minCoords[j].w) == localCollisionPairs[collisionPairIdx].x && int(minCoords[i].w) == localCollisionPairs[collisionPairIdx].y)) &&
			  minCoords[i].y < maxCoords[j].y && maxCoords[i].y > minCoords[j].y)
		{
			uint availableIdx = atomicCounterIncrement(nextAvailableIdx);
			collisionPairs[availableIdx].x = int(minCoords[i].w); // w comp stores object ID
			collisionPairs[availableIdx].y = int(minCoords[j].w);

			break;
		}
	}
}

subroutine(sweep)
void sweepZ(uint i, uint j)
{
	for (int collisionPairIdx = 0; collisionPairIdx < 2 * currNumColliders; ++collisionPairIdx)
	{
		if (((int(minCoords[i].w) == localCollisionPairs[collisionPairIdx].x && int(minCoords[j].w) == localCollisionPairs[collisionPairIdx].y) ||
			 (int(minCoords[j].w) == localCollisionPairs[collisionPairIdx].x && int(minCoords[i].w) == localCollisionPairs[collisionPairIdx].y)) &&
			  minCoords[i].z < maxCoords[j].z && maxCoords[i].z > minCoords[j].z)
		{
			uint availableIdx = atomicCounterIncrement(nextAvailableIdx);

			// Sort ID for consistency
			if (minCoords[i].w < minCoords[j].w)
			{
				collisionPairs[availableIdx].x = int(minCoords[j].w); // w comp stores object ID
				collisionPairs[availableIdx].y = int(minCoords[i].w);
			}
			else
			{
				collisionPairs[availableIdx].x = int(minCoords[i].w);
				collisionPairs[availableIdx].y = int(minCoords[j].w);
			}

			break;
		}
	}
}

// void prune()
// {
// 	uint aabbIdx = gl_LocalInvocationID.x;

// 	if (aabbIdx >= currNumColliders) return;

// 	for ( uint collisionPairIdx = 0
// 		; collisionPairIdx < atomicCounter(nextAvailableIdx)
// 		; ++collisionPairIdx )
// 	{
// 		if (minCoords[aabbIdx].w < 1000.0f &&
// 			(collisionPairs[collisionPairIdx].x == minCoords[aabbIdx].w ||
// 			collisionPairs[collisionPairIdx].y == minCoords[aabbIdx].w))
// 		{
// 			minCoords[aabbIdx].w += 1000.0f; // Update the AABB list
// 		}
// 	}
// }

void main()
{
	uint collisionPairIdx = 2 * gl_LocalInvocationID.x;

	// Reset collision pair buffer
	for ( uint collisionPairCycle = collisionPairIdx
		; collisionPairCycle < collisionPairIdx + 2u && collisionPairCycle < 2 * MAX_NUM_COLLIDERS
		; ++collisionPairCycle )
	{
		localCollisionPairs[collisionPairCycle] = collisionPairs[collisionPairCycle].xy;

		// Reset
		collisionPairs[collisionPairCycle] = ivec4(-5, -5, -5, -5);
	}

	barrier(); //=============================================================
	memoryBarrierShared();
	memoryBarrier();

	uint currObjectIdx = gl_LocalInvocationID.x;

	// Still loop through pretty much all objects in the world
	for (uint nextObjectIdx = 0u; nextObjectIdx < currNumColliders; ++nextObjectIdx)
	{
		if (nextObjectIdx == currObjectIdx) continue;

		sweepAxis(currObjectIdx, nextObjectIdx); // Collision pair buffer got updated.
	}

	barrier(); //=============================================================

	// prune();
}
	)sap";

//===========================================================================//

	constexpr char evenOddSort[] = R"evenOddSort(
#version 430 core

#define MAX_NUM_COLLIDERS 1024

layout(local_size_x = MAX_NUM_COLLIDERS) in;

layout(std430, binding = 1) volatile buffer unsorted_data
{
	vec4 minCoords[MAX_NUM_COLLIDERS];
	vec4 maxCoords[MAX_NUM_COLLIDERS];
};

uniform uint currNumColliders;
uniform int evenOrOdd;

void swap(uint, uint);

subroutine void sort(uint i, uint j);

subroutine uniform sort sortComponent;

subroutine(sort)
void sortX(uint i, uint j)
{
	if (minCoords[i].x > minCoords[j].x)
		swap(i, j);
}

subroutine(sort)
void sortY(uint i, uint j)
{
	if (minCoords[i].y > minCoords[j].y)
		swap(i, j);
}

subroutine(sort)
void sortZ(uint i, uint j)
{
	if (minCoords[i].z > minCoords[j].z)
	{
		swap(i, j);
	}
}

void swap(uint i, uint j)
{
	vec4 temp = minCoords[i];
	minCoords[i] = minCoords[j];
	minCoords[j] = temp;

	temp = maxCoords[i];
	maxCoords[i] = maxCoords[j];
	maxCoords[j] = temp;
}

void main()
{
	// Figure what index pair we are looking at
	uint i = gl_LocalInvocationID.x;
	uint j = gl_LocalInvocationID.x + 1;

	// If either index is bigger than the buffer size, we let the compute unit idle
	if (i >= currNumColliders || j >= currNumColliders) return;

	//--------------- Check to see if we are doing odd pair or even pair ---------------//
	// If we are doing odd, the starting index must be even
	if (evenOrOdd == 1 && i % 2 != 0) return;

	// If we are doing even, the starting index must be odd
	if (evenOrOdd == -1 && i % 2 == 0) return;

	//-------------------- The actual sweeping --------------------//
	sortComponent(i, j);

	barrier();
}
	)evenOddSort";

//===========================================================================//

	constexpr char sat[] = R"sat(
#version 430

precision highp float;

// Properties of the collider should be more configurable
#define VERT_COUNT_PER_EDGE 2
#define VERT_COUNT_PER_FACE 4
#define COLLIDER_EDGE_COUNT 12
#define COLLIDER_FACE_COUNT 6
#define COLLIDER_VERT_COUNT 8
#define MAX_CONTACT_POINT_COUNT 16
#define MAX_COLLIDER_COUNT 1024
#define FLT_MAX 9999.9f
#define EPSILON 0.0001f

layout(local_size_x = MAX_COLLIDER_COUNT) in;

const int faces[COLLIDER_FACE_COUNT][VERT_COUNT_PER_FACE] =
{
	{ 0, 3, 2, 1 }, // front
	{ 0, 4, 7, 3 }, // left
	{ 4, 5, 6, 7 }, // back
	{ 5, 1, 2, 6 }, // right
	{ 0, 1, 5, 4 }, // top
	{ 3, 7, 6, 2 }  // bottom
};

const int edges[COLLIDER_EDGE_COUNT][VERT_COUNT_PER_EDGE] =
{
	{ 0, 1 }, { 1, 5 }, { 4, 7 },
	{ 0, 3 }, { 2, 6 }, { 7, 6 },
	{ 3, 2 }, { 6, 5 }, { 4, 0 },
	{ 2, 1 }, { 4, 5 }, { 3, 7 }
};

struct BoxCollider
{
	vec4 vertices[COLLIDER_VERT_COUNT];
};

struct Manifold
{
	ivec4 contactBoxIndicesAndContactCount;
	vec4 contactPoints[MAX_CONTACT_POINT_COUNT];
	vec4 contactNormal;
};

struct Plane
{
	vec3 point;
	vec3 normal;
};

struct FaceQuery
{
	int faceIdx;
	float largestDist;
	vec3 faceNormal;
};

struct EdgeQuery
{
	float largestDist;
	vec3 pointsA[VERT_COUNT_PER_EDGE];
	vec3 pointsB[VERT_COUNT_PER_EDGE];
	vec3 edgeDirA;
	vec3 edgeDirB;
	vec3 edgeNormal; // This edge normal points away from edgeA.
};

/*-------------------------- Buffers ------------------------------*/
layout(std430, binding = 0) readonly buffer in_collider_data
{
	ivec4 colliderMisc;
	BoxCollider boxColliders[];
};

layout(std430, binding = 1) readonly buffer in_collision_pair_data
{
	ivec4 collisionPairMisc;
	ivec4 collisionPairs[2 * MAX_COLLIDER_COUNT];
};

layout(std430, binding = 2) writeonly buffer out_manifolds_data
{
	ivec4 manifoldMisc;
	Manifold manifolds[MAX_COLLIDER_COUNT];
};

//layout(std430, binding = 3) writeonly buffer experimental_out_buffer
//{
//	// Pay attention to the memory layout (offsets in bytes): [0, 16]
//	ivec4 misc;
//	// [16 - sizeof(ivec4) * MAX_COLLIDER_COUNT] = [16 - 16 + 16 * 1024] = [16, 16,400]
//	ivec4 contactBoxIndicesAndContactCount[MAX_COLLIDER_COUNT]; // x = refBoxIdx, y = incidentBoxIdx, z = contact count
//	// [16,400 - 16,400 + sizeof(vec4) * MAX_CONTACT_POINT_COUNT * MAX_COLLIDER_COUNT] = [16,400 - 278,544]
//	vec4 contactPoints[MAX_COLLIDER_COUNT][MAX_CONTACT_POINT_COUNT];
//	// [278,544 - 278,544 + sizeof(vec4) * MAX_COLLIDER_COUNT] = [278,544 - 294,928]
//	vec4 contactNormal[MAX_COLLIDER_COUNT]; // w stores the penetration depth.
//} experimentalBuffer;
//

layout(binding = 0) uniform atomic_uint nextAvailableIdx;
/*-----------------------------------------------------------------*/

float getSignedDist(vec3 point, Plane plane)
{
	return dot(plane.normal, point - plane.point);
}

vec3 projectPointOntoPlane(vec3 point, Plane plane)
{
	return point - getSignedDist(point, plane) * plane.normal;
}

vec3 getSupport(BoxCollider box, vec3 direction)
{
	float projDist = 0.0f;
	float largestProjDist = -FLT_MAX;
	vec3 supportPoint = vec3(0.0f);

	for (int vertIdx = 0; vertIdx < COLLIDER_VERT_COUNT; ++vertIdx)
	{
		projDist = dot(box.vertices[vertIdx].xyz, direction);

		if (projDist > largestProjDist)
		{
			supportPoint = box.vertices[vertIdx].xyz;
			largestProjDist = projDist;
		}
	}

	return supportPoint;
}

vec3 getFaceNormal(BoxCollider box, int faceIdx)
{
	vec3 a = box.vertices[faces[faceIdx][0]].xyz;
	vec3 b = box.vertices[faces[faceIdx][1]].xyz;
	vec3 c = box.vertices[faces[faceIdx][2]].xyz;

	return normalize(cross(b - a, c - a));
}

Plane getPlane(BoxCollider box, int faceIdx)
{
	Plane plane;
	plane.point = box.vertices[faces[faceIdx][0]].xyz;
	plane.normal = getFaceNormal(box, faceIdx);

	return plane;
}

FaceQuery queryFaceDirections(BoxCollider boxA, BoxCollider boxB)
{
	float dist = 0.0f;
	Plane plane;
	vec3 supportPoint = vec3(0.0f);
	FaceQuery faceQuery;

	faceQuery.faceIdx = -1;
	faceQuery.largestDist = -FLT_MAX;

	for (int localFaceIdx = 0; localFaceIdx < COLLIDER_FACE_COUNT; ++localFaceIdx)
	{
		plane = getPlane(boxA, localFaceIdx);

		supportPoint = getSupport(boxB, -plane.normal);

		dist = getSignedDist(supportPoint, plane);

		if (dist > faceQuery.largestDist)
		{
			faceQuery.faceIdx = localFaceIdx;
			faceQuery.largestDist = dist;
		}
	}

	return faceQuery;
}

EdgeQuery queryEdgeDirections(BoxCollider boxA, BoxCollider boxB)
{
	vec3 startA = vec3(0.0f);
	vec3 endA = vec3(0.0f);
	vec3 startB = vec3(0.0f);
	vec3 endB = vec3(0.0f);
	vec3 edgeA = vec3(0.0f);
	vec3 edgeB = vec3(0.0f);
	vec3 edgeNormal = vec3(0.0f);
	vec3 supportPointB = vec3(0.0f);
	float localDist = 0.0f;
	Plane plane;
	EdgeQuery edgeQuery;

	edgeQuery.largestDist = -FLT_MAX;

	vec3 centerA = 0.5f * (boxA.vertices[5].xyz + boxA.vertices[3].xyz);

	for (int edgeIdxA = 0; edgeIdxA < COLLIDER_EDGE_COUNT; ++edgeIdxA)
	{
		startA = boxA.vertices[edges[edgeIdxA][0]].xyz;
		endA = boxA.vertices[edges[edgeIdxA][1]].xyz;
		edgeA = endA - startA;

		for (int edgeIdxB = 0; edgeIdxB < COLLIDER_EDGE_COUNT; ++edgeIdxB)
		{
			startB = boxB.vertices[edges[edgeIdxB][0]].xyz;
			endB = boxB.vertices[edges[edgeIdxB][1]].xyz;
			edgeB = endB - startB;

			edgeNormal = normalize(cross(edgeA, edgeB));

			if (dot(edgeNormal, startA - centerA) < 0.0f)
				edgeNormal *= -1.0f;

			plane.point = startA;
			plane.normal = edgeNormal;
			supportPointB = getSupport(boxB, -edgeNormal);
			localDist = getSignedDist(supportPointB, plane);

			if (localDist > edgeQuery.largestDist)
			{
				edgeQuery.largestDist = localDist;
				edgeQuery.pointsA = vec3[2](startA, endA);
				edgeQuery.pointsB = vec3[2](startB, endB);
				edgeQuery.edgeDirA = edgeA;
				edgeQuery.edgeDirB = edgeB;
				edgeQuery.edgeNormal = edgeNormal;
			}
		}
	}

	return edgeQuery;
}

int getIncidentFaceIdx(BoxCollider incidentBox, vec3 referenceNormal)
{
	int incidentFaceIdx = -1;
	vec3 incidentNormal = vec3(0.0f);
	float smallestDotProduct = FLT_MAX;
	float localDotProduct = 0.0f;

	for (int faceIdx = 0; faceIdx < COLLIDER_FACE_COUNT; ++faceIdx)
	{
		incidentNormal = getFaceNormal(incidentBox, faceIdx);
		localDotProduct = dot(incidentNormal, referenceNormal);

		if (localDotProduct < smallestDotProduct)
		{
			smallestDotProduct = localDotProduct;
			incidentFaceIdx = faceIdx;
		}
	}

	return incidentFaceIdx;
}

void reduceContactPoints(inout Manifold manifold)
{
	int firstContactIdx = -1;
	float largestSeparation = -FLT_MAX;

	for (int i = 0; i < manifold.contactBoxIndicesAndContactCount.z; ++i)
	{
		float separation = dot(manifold.contactPoints[i].xyz, -manifold.contactNormal.xyz);

		if (separation > largestSeparation)
		{
			largestSeparation = separation;
			firstContactIdx = i;
		}
	}

	vec3 a = manifold.contactPoints[firstContactIdx].xyz;

	int secondContactIdx = -1;
	largestSeparation = -FLT_MAX;

	for (int j = 0; j < manifold.contactBoxIndicesAndContactCount.z; ++j)
	{
		if (j == firstContactIdx) continue;

		float separation = length(manifold.contactPoints[j].xyz - a);

		if (separation > largestSeparation)
		{
			largestSeparation = separation;
			secondContactIdx = j;
		}
	}

	vec3 b = vec3(manifold.contactPoints[secondContactIdx]);

	int thirdContactIdx = -1;
	float largestArea = -FLT_MAX;

	for (int k = 0; k < manifold.contactBoxIndicesAndContactCount.z; ++k)
	{
		if (k == firstContactIdx || k == secondContactIdx) continue;

		vec3 potC = manifold.contactPoints[k].xyz;
		float area = 0.5f * dot(cross(a - potC, b - potC), vec3(manifold.contactNormal));

		if (area > largestArea)
		{
			largestArea = area;
			thirdContactIdx = k;
		}
	}

	vec3 c = manifold.contactPoints[thirdContactIdx].xyz;

	int fourthContactIdx = -1;
	largestArea = FLT_MAX;

	for (int l = 0; l < manifold.contactBoxIndicesAndContactCount.z; ++l)
	{
		if (l == firstContactIdx || l == secondContactIdx || l == thirdContactIdx) continue;

		vec3 potD = manifold.contactPoints[l].xyz;
		float area = 0.5f * dot(cross(a - potD, b - potD), manifold.contactNormal.xyz);

		if (area < largestArea)
		{
			largestArea = area;
			fourthContactIdx = l;
		}
	}

	vec3 d = manifold.contactPoints[fourthContactIdx].xyz;

	manifold.contactPoints[0] = vec4(a, 1.0f);
	manifold.contactPoints[1] = vec4(b, 1.0f);
	manifold.contactPoints[2] = vec4(c, 1.0f);
	manifold.contactPoints[3] = vec4(d, 1.0f);
	manifold.contactBoxIndicesAndContactCount.z = 4;
}

Manifold createFaceContact(FaceQuery faceQueryA, FaceQuery faceQueryB,
							BoxCollider boxA, BoxCollider boxB,
							int boxAIdx, int boxBIdx)
{
	int referenceBoxIdx = -1;
	int incidentBoxIdx = -1;
	int incidentFaceIdx = -1;
	float referenceSeparation = 0.0f;
	Plane referencePlane;
	BoxCollider incidentBox;
	BoxCollider referenceBox;
	const float axisBias = 0.4f;

	if (axisBias * faceQueryA.largestDist > faceQueryB.largestDist)
	{
		referencePlane = getPlane(boxA, faceQueryA.faceIdx);
		referenceBoxIdx = boxAIdx;
		referenceBox = boxA;
		referenceSeparation = faceQueryA.largestDist;

		incidentFaceIdx = getIncidentFaceIdx(boxB, referencePlane.normal);
		incidentBoxIdx = boxBIdx;
		incidentBox = boxB;
	}
	else
	{
		referencePlane = getPlane(boxB, faceQueryB.faceIdx);
		referenceBoxIdx = boxBIdx;
		referenceBox = boxB;
		referenceSeparation = faceQueryB.largestDist;

		incidentFaceIdx = getIncidentFaceIdx(boxA, referencePlane.normal);
		incidentBoxIdx = boxAIdx;
		incidentBox = boxA;
	}

	Plane clipPlane;
	vec3 startVert = vec3(0.0f);
	vec3 endVert = vec3(0.0f);
	vec3 lerpIntersectPoint = vec3(0.0f);
	vec3 projPointOntoRefPlane = vec3(0.0f);
	int startVertIdx = -1;
	int endVertIdx = -1;
	float startSignedDist = 0.0f;
	float endSignedDist = 0.0f;
	float lerpRatio = 0.0f;
	int contactPointCount = 0;

	Manifold manifold;

	manifold.contactBoxIndicesAndContactCount = ivec4(0);

	for (int i = 0; i < MAX_CONTACT_POINT_COUNT; ++i)
	{
		manifold.contactPoints[i] = vec4(0.0f);
	}

	manifold.contactNormal = vec4(0.0f);

	for (int faceIdx = 0; faceIdx < COLLIDER_FACE_COUNT; ++faceIdx)
	{
		clipPlane = getPlane(referenceBox, faceIdx);
		if (abs(dot(referencePlane.normal, clipPlane.normal)) <= EPSILON)
		{
			startVertIdx = faces[incidentFaceIdx][0];
			startVert = vec3(incidentBox.vertices[startVertIdx]);

			for (int vertIdx = 1; vertIdx < VERT_COUNT_PER_FACE; ++vertIdx)
			{
				if (contactPointCount >= MAX_CONTACT_POINT_COUNT) break;

				endVertIdx = faces[incidentFaceIdx][vertIdx];
				endVert = vec3(incidentBox.vertices[endVertIdx]);

				startSignedDist = getSignedDist(startVert, clipPlane);
				endSignedDist = getSignedDist(endVert, clipPlane);

				if (sign(startSignedDist) == 1.0f && sign(endSignedDist) == -1.0f)
				{
					if (sign(getSignedDist(endVert, referencePlane)) == -1.0f)
					{
						projPointOntoRefPlane = projectPointOntoPlane(endVert, referencePlane);
						manifold.contactPoints[contactPointCount++] = vec4(projPointOntoRefPlane, 0.0f);
					}

					lerpRatio = startSignedDist / (startSignedDist - endSignedDist);
					lerpIntersectPoint = mix(startVert, endVert, lerpRatio);

					if (sign(getSignedDist(lerpIntersectPoint, referencePlane)) == -1.0f)
					{
						projPointOntoRefPlane = projectPointOntoPlane(lerpIntersectPoint, referencePlane);
						manifold.contactPoints[contactPointCount++] = vec4(projPointOntoRefPlane, 0.0f);
					}
				}

				else if (sign(startSignedDist) == -1.0f && sign(endSignedDist) == -1.0f
							&& sign(getSignedDist(endVert, referencePlane)) < -1.0f)
				{
					projPointOntoRefPlane = projectPointOntoPlane(endVert, referencePlane);
					manifold.contactPoints[contactPointCount++] = vec4(projPointOntoRefPlane, 0.0f);
				}

				else if (sign(startSignedDist) == -1.0f && sign(endSignedDist) == 1.0f)
				{
					lerpRatio = startSignedDist / (startSignedDist - endSignedDist);
					lerpIntersectPoint = mix(startVert, endVert, lerpRatio);

					if (sign(getSignedDist(lerpIntersectPoint, referencePlane)) == -1.0f)
					{
						projPointOntoRefPlane = projectPointOntoPlane(lerpIntersectPoint, referencePlane);
						manifold.contactPoints[contactPointCount++] = vec4(projPointOntoRefPlane, 0.0f);
					}
				}

				startVert = endVert;
			}
		}
	}

	manifold.contactBoxIndicesAndContactCount.x = referenceBoxIdx;
	manifold.contactBoxIndicesAndContactCount.y = incidentBoxIdx;
	manifold.contactBoxIndicesAndContactCount.z = contactPointCount;
	manifold.contactNormal = vec4(referencePlane.normal, referenceSeparation);

	if (contactPointCount > 4)
	{
		reduceContactPoints(manifold);
	}

	return manifold;
}

Manifold createEdgeContact(EdgeQuery edgeQuery, int boxAIdx, int boxBIdx)
{
	Manifold manifold;

	manifold.contactBoxIndicesAndContactCount = ivec4(0);

	for (int i = 0; i < MAX_CONTACT_POINT_COUNT; ++i)
	{
		manifold.contactPoints[i] = vec4(0.0f);
	}

	manifold.contactNormal = vec4(0.0f);

	float s = 0, t = 0;

	vec3 r = edgeQuery.pointsA[0] - edgeQuery.pointsB[0];
	float squaredLengthEdgeA = dot(edgeQuery.edgeDirA, edgeQuery.edgeDirA);
	float squaredLengthEdgeB = dot(edgeQuery.edgeDirB, edgeQuery.edgeDirB);
	float f = dot(edgeQuery.edgeDirB, r);

	float c = dot(edgeQuery.edgeDirA, r);
	float b = dot(edgeQuery.edgeDirA, edgeQuery.edgeDirB);
	float denom = squaredLengthEdgeA * squaredLengthEdgeB - b * b;

	if (denom != 0.0f)
		s = clamp((b * f - c * squaredLengthEdgeB) / denom, 0.0f, 1.0f);

	t = (b * s + f) / squaredLengthEdgeB;

	if (t < 0.0f)
	{
		t = 0.0f;
		s = clamp(-c / squaredLengthEdgeA, 0.0f, 1.0f);
	}
	else if (t > 1.0f)
	{
		t = 1.0f;
		s = clamp((b - c) / squaredLengthEdgeA, 0.0f, 1.0f);
	}

	vec3 closestPointA = edgeQuery.pointsA[0] + s * edgeQuery.edgeDirA;
	vec3 closestPointB = edgeQuery.pointsB[0] + t * edgeQuery.edgeDirB;

	manifold.contactPoints[0] = vec4(0.5f * (closestPointB + closestPointA), 0.0f);

	manifold.contactBoxIndicesAndContactCount.x = boxAIdx;
	manifold.contactBoxIndicesAndContactCount.y = boxBIdx;
	manifold.contactBoxIndicesAndContactCount.z = 1;
	manifold.contactNormal = vec4(edgeQuery.edgeNormal, edgeQuery.largestDist);

	return manifold;
}

void separatingAxisTest(uint collisionPairIdx)
{
	int boxAIdx = collisionPairs[collisionPairIdx].x;
	int boxBIdx = collisionPairs[collisionPairIdx].y;
	BoxCollider boxA = boxColliders[boxAIdx];
	BoxCollider boxB = boxColliders[boxBIdx];
	const float queryBias = 0.5f;

	FaceQuery faceQueryA = queryFaceDirections(boxA, boxB);
	if (faceQueryA.largestDist > 0.0f) return;

	FaceQuery faceQueryB = queryFaceDirections(boxB, boxA);
	if (faceQueryB.largestDist > 0.0f) return;

	EdgeQuery edgeQuery = queryEdgeDirections(boxA, boxB);
	if (edgeQuery.largestDist > 0.0f) return;

	Manifold manifold;

	if (queryBias * faceQueryA.largestDist > edgeQuery.largestDist
		&& queryBias * faceQueryB.largestDist > edgeQuery.largestDist)
	{
		manifold = createFaceContact(faceQueryA, faceQueryB, boxA, boxB, boxAIdx, boxBIdx);
	}
	else
	{
		manifold = createEdgeContact(edgeQuery, boxAIdx, boxBIdx);
	}

	uint availableIdx = atomicCounterIncrement(nextAvailableIdx);
	manifolds[availableIdx] = manifold;
}

void main()
{
	uint collisionPairIdx = gl_GlobalInvocationID.x;

	if (collisionPairs[collisionPairIdx].x >= 0)
	{
		separatingAxisTest(collisionPairIdx);
	}
}
	)sat";
}


GLuint createComputeProgram(std::string const &shaderName)
{
	// Load the compute shader source code
	const char *shader;
	std::string shaderSource;

	if (shaderName == "../resources/shaders/updateAabbs.comp")
	{
		shader = shadersource::updateAabbs;
	}
	else if (shaderName == "../resources/shaders/evenOddSort.comp")
	{
		shader = shadersource::evenOddSort;
	}
	else if (shaderName == "../resources/shaders/sap.comp")
	{
		shader = shadersource::sap;
	}
	else if (shaderName == "../resources/shaders/sat.comp")
	{
		shader = shadersource::sat;
	}
	else
	{
		shaderSource = oglutils::readFileAsString(shaderName);
		shader = shaderSource.c_str();
	}

	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &shader, nullptr);

	GLint success = 0;
	CHECKED_GL_CALL(glCompileShader(computeShader));

	// Check for compile status
	CHECKED_GL_CALL(glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success));
	if (!success)
	{
		GLSL::printShaderInfoLog(computeShader);
		CHECKED_GL_CALL(glDeleteShader(computeShader));
		std::cerr << "Error compiling compute shader " + shaderName + ". Shader object will be deleted.\n";

		throw std::runtime_error("Error compiling compute shader " + shaderName + ". Shader object will be deleted.");
	}

	GLuint programID = glCreateProgram();
	CHECKED_GL_CALL(glAttachShader(programID, computeShader));
	CHECKED_GL_CALL(glLinkProgram(programID));

	// Check for linking status
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar programInfoCStr[1000];
		GLsizei programInfoCStrLength = 0u;
		glGetProgramInfoLog(programID, 1000, &programInfoCStrLength, programInfoCStr);

		std::cerr << programInfoCStr << '\n';

		GLSL::printShaderInfoLog(computeShader);
		CHECKED_GL_CALL(glDetachShader(programID, computeShader));
		CHECKED_GL_CALL(glDeleteShader(computeShader));
		std::cerr << "Error linking compute shader " + shaderName + ". Shader object will be deleted.\n";
		throw std::runtime_error("Error linking compute shader " + shaderName + ". Shader object will be deleted.");
	}

	// Detach and delete compute shader after linking
	CHECKED_GL_CALL(glDetachShader(programID, computeShader));
	CHECKED_GL_CALL(glDeleteShader(computeShader));

	return programID;
}
