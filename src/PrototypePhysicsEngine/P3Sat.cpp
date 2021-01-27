#include "P3Sat.h"

#include <array>

#include "P3BroadPhaseCommon.h"
#include "P3NarrowPhaseCommon.h"

constexpr int cVertCountPerEdge  =  2;
constexpr int cVertCountPerFace  =  4;
constexpr int cColliderEdgeCount = 12;
constexpr int cColliderFaceCount =  6;
constexpr int cColliderVertCount =  8;

using BoxCollider = glm::vec4 const *; // A constant array
using ColliderFaceNormals = std::array<std::array<glm::vec3, cColliderFaceCount>, cMaxColliderCount>;

constexpr int cFaces[cColliderFaceCount][cVertCountPerFace] =
{
	{ 0, 3, 2, 1 }, // front
	{ 0, 4, 7, 3 }, // left
	{ 4, 5, 6, 7 }, // back
	{ 5, 1, 2, 6 }, // right
	{ 0, 1, 5, 4 }, // top
	{ 3, 7, 6, 2 }  // bottom
};

constexpr int cEdges[cColliderEdgeCount][cVertCountPerEdge] =
{
	{ 0, 1 }, { 1, 5 }, { 4, 7 },
	{ 0, 3 }, { 2, 6 }, { 7, 6 },
	{ 3, 2 }, { 6, 5 }, { 4, 0 },
	{ 2, 1 }, { 4, 5 }, { 3, 7 }
};

struct Plane
{
	glm::vec3 point{ 0.0f };
	glm::vec3 normal{ 0.0f };
};

struct FaceQuery
{
	int faceIdx{ -1 };
	float largestDist{ -9999.9f };
	glm::vec3 faceNormal{ 0.0f };
};

struct EdgeQuery
{
	float largestDist{ -9999.9f };
	glm::vec3 pointsA[cVertCountPerEdge]{};
	glm::vec3 pointsB[cVertCountPerEdge]{};
	glm::vec3 edgeDirA{ 0.0f };
	glm::vec3 edgeDirB{ 0.0f };
	glm::vec3 edgeNormal{ 0.0f };
};

float getSignedDist(glm::vec3 const &point, Plane const &plane)
{
	return glm::dot(plane.normal, point - plane.point);
}

// Assume planeNormal is normalized.
glm::vec3 projectPointOntoPlane(glm::vec3 const &point, Plane const &plane)
{
	return point + getSignedDist(point, plane) * plane.normal;
}

glm::vec3 getSupport(BoxCollider box, glm::vec3 const &direction)
{
	float projDist        = 0.0f;
	float largestProjDist = 0.0f;
	glm::vec3 supportPoint{ 0.0f };
	glm::vec3 vertPos{ 0.0f };

	for (int vertIdx = 0; vertIdx < cColliderVertCount; ++vertIdx)
	{
		vertPos  = box[vertIdx];
		projDist = glm::dot(vertPos, direction);

		if (projDist > largestProjDist)
		{
			supportPoint    = vertPos;
			largestProjDist = projDist;
		}
	}

	return supportPoint;
}

glm::vec3 getFaceNormal(BoxCollider box, int boxIdx, int faceIdx)
{
	glm::vec3 a{ box[cFaces[faceIdx][0]] };
	glm::vec3 b{ box[cFaces[faceIdx][1]] };
	glm::vec3 c{ box[cFaces[faceIdx][2]] };

	return glm::normalize(glm::cross(b - a, c - a));
}

Plane getPlane( BoxColliderGpuPackage const &boxColliderPackage, int boxIdx,
				ColliderFaceNormals const &colliderFaceNormals, int faceIdx )
{
	Plane plane;
	plane.point  = colliderFaceNormals[boxIdx][faceIdx];
	plane.normal = boxColliderPackage[boxIdx][cFaces[faceIdx][0]];

	return plane;
}

FaceQuery queryFaceDirections( BoxColliderGpuPackage const &boxColliderPackage,
							   int boxAIdx, int boxBIdx,
							   ColliderFaceNormals const &colliderFaceNormals )
{
	float dist = 0.0f;
	Plane plane;
	glm::vec3 supportPoint{ 0.0f };
	FaceQuery faceQuery;

	for (int localFaceIdx = 0; localFaceIdx < cColliderFaceCount; ++localFaceIdx)
	{
		plane = getPlane(boxColliderPackage, boxAIdx, colliderFaceNormals, localFaceIdx);
		supportPoint = getSupport(boxColliderPackage[boxBIdx], -plane.normal);
		dist = getSignedDist(supportPoint, plane);

		if (dist > faceQuery.largestDist)
		{
			faceQuery.faceIdx     = localFaceIdx;
			faceQuery.largestDist = dist;
		}
	}

	return faceQuery;
}

EdgeQuery queryEdgeDirections(BoxCollider boxA, BoxCollider boxB)
{
	glm::vec3 startA{ 0.0f };
	glm::vec3 endA{ 0.0f };
	glm::vec3 startB{ 0.0f };
	glm::vec3 endB{ 0.0f };
	glm::vec3 edgeA{ 0.0f };
	glm::vec3 edgeB{ 0.0f };
	glm::vec3 edgeNormal{ 0.0f };
	glm::vec3 supportPointB{ 0.0f };
	float localDist = 0.0f;
	Plane plane;
	EdgeQuery edgeQuery;

	glm::vec3 centerA = 0.5f * (boxA[5] + boxA[3]);

	for (int edgeIdxA = 0; edgeIdxA < cColliderEdgeCount; ++edgeIdxA)
	{
		startA = boxA[cEdges[edgeIdxA][0]];
		endA   = boxA[cEdges[edgeIdxA][1]];
		edgeA  = endA - startA;

		for (int edgeIdxB = 0; edgeIdxB < cColliderEdgeCount; ++edgeIdxB)
		{
			startB = boxB[cEdges[edgeIdxB][0]];
			endB = boxB[cEdges[edgeIdxB][1]];
			edgeB = endB - startB;

			edgeNormal = glm::normalize(glm::cross(edgeA, edgeB));

			if (glm::dot(edgeNormal, startA - centerA) < 0.0f)
				edgeNormal *= -1.0f;

			plane.point   = startA;
			plane.normal  = edgeNormal;
			supportPointB = getSupport(boxB, -edgeNormal);
			localDist     = getSignedDist(supportPointB, plane);

			if (localDist > edgeQuery.largestDist)
			{
				edgeQuery.largestDist = localDist;
				edgeQuery.pointsA[0]  = startA;
				edgeQuery.pointsA[1]  = endA;
				edgeQuery.pointsB[0]  = startB;
				edgeQuery.pointsB[1]  = endB;
				edgeQuery.edgeDirA    = edgeA;
				edgeQuery.edgeDirB    = edgeB;
				edgeQuery.edgeNormal  = edgeNormal;
			}
		}
	}

	return edgeQuery;
}

glm::vec3 getIncidentNormal( BoxColliderGpuPackage const &boxColliderPackage, int incidentBoxIdx,
							 ColliderFaceNormals const &colliderFaceNormals,
							 glm::vec3 const &referenceNormal )
{
	glm::vec3 incidentNormal{ 0.0f };
	glm::vec3 potentialIncidentNormal{ 0.0f };
	float smallestDotProduct = 9999.9f;
	float localDotProduct    = 0.0f;

	for (int faceIdx = 0; faceIdx < cColliderFaceCount; ++faceIdx)
	{
		potentialIncidentNormal = colliderFaceNormals[incidentBoxIdx][faceIdx];
		localDotProduct         = glm::dot(potentialIncidentNormal, referenceNormal);
	}

	if (localDotProduct < smallestDotProduct)
	{
		smallestDotProduct = localDotProduct;
		incidentNormal     = potentialIncidentNormal;
	}

	return incidentNormal;
}

Manifold createFaceContact( FaceQuery const &faceQueryA, FaceQuery const &faceQueryB,
							BoxColliderGpuPackage const &boxColliderPackage,
							int boxAIdx, int boxBIdx,
							ColliderFaceNormals const &colliderFaceNormals )
{
	glm::vec3 referencePoint{ 0.0f };
	glm::vec3 referenceNormal{ 0.0f };
	glm::vec3 incidentNormal{ 0.0f };
	int referenceBoxIdx = -1;
	int incidentBoxIdx  = -1;
	float incidentLargestDist = 0.0f;
	Plane referencePlane;

	if (faceQueryA.largestDist < faceQueryB.largestDist)
	{
		referencePlane  = getPlane(boxColliderPackage, boxAIdx, colliderFaceNormals, faceQueryA.faceIdx);
		referenceBoxIdx = boxAIdx;

		incidentNormal = getIncidentNormal(boxColliderPackage, boxBIdx, colliderFaceNormals, referenceNormal);
		incidentBoxIdx = boxBIdx;
		incidentLargestDist = faceQueryB.largestDist;
	}
	else
	{
		referencePlane  = getPlane(boxColliderPackage, boxBIdx, colliderFaceNormals, faceQueryB.faceIdx);
		referenceBoxIdx = boxBIdx;

		incidentNormal = getIncidentNormal(boxColliderPackage, boxAIdx, colliderFaceNormals, referenceNormal);
		incidentBoxIdx = boxAIdx;
		incidentLargestDist = faceQueryA.largestDist;

		// THIS IS SO FREAKING DUMB! Gotta keep it PG.
	}

	BoxCollider incidentBox = boxColliderPackage[incidentBoxIdx];
	Plane clipPlane;
	glm::vec3 startVert{ 0.0f };
	glm::vec3 endVert{ 0.0f };
	glm::vec3 lerpIntersectPoint{ 0.0f };
	glm::vec3 projPointOntoRefPlane{ 0.0f };
	int startVertIdx = -1;
	int endVertIdx   = -1;
	float startSignedDist = 0.0f;
	float endSignedDist   = 0.0f;
	float lerpRatio       = 0.0f;
	int contactPointCount = 0;
	Manifold manifold;

	for (int faceIdx = 0; faceIdx < cColliderFaceCount; ++faceIdx)
	{
		clipPlane = getPlane(boxColliderPackage, referenceBoxIdx, colliderFaceNormals, faceIdx);
		if (glm::dot(referenceNormal, clipPlane.normal) < 0.0001f)
		{
			startVertIdx = cFaces[faceIdx][0];
			startVert    = incidentBox[startVertIdx];

			for (int vertIdx = 1; vertIdx < cVertCountPerFace; ++vertIdx)
			{
				if (contactPointCount >= cMaxContactPointCount) break;

				endVertIdx = cFaces[faceIdx][vertIdx];
				endVert  = incidentBox[endVertIdx];

				startSignedDist = getSignedDist(startVert, clipPlane);
				endSignedDist   = getSignedDist(endVert, clipPlane);

				if (startSignedDist > 0.0001f && endSignedDist < -0.0001f)
				{
					if (getSignedDist(endVert, referencePlane) < -0.0001f)
					{
						projPointOntoRefPlane = projectPointOntoPlane(endVert, referencePlane);
						manifold.contactPoints[contactPointCount++] = glm::vec4(projPointOntoRefPlane, 0.0f);
					}

					lerpRatio = startSignedDist / (startSignedDist - endSignedDist);
					lerpIntersectPoint = glm::mix(startVert, endVert, lerpRatio);

					if (getSignedDist(lerpIntersectPoint, referencePlane) < -0.0001f)
					{
						projPointOntoRefPlane = projectPointOntoPlane(lerpIntersectPoint, referencePlane);
						manifold.contactPoints[contactPointCount++] = glm::vec4(projPointOntoRefPlane, 0.0f);
					}
				}
				else if ( startSignedDist < -0.0001f && endSignedDist < -0.0001f
						  && getSignedDist(endVert, referencePlane) < -0.0001f )
				{
					projPointOntoRefPlane = projectPointOntoPlane(endVert, referencePlane);
					manifold.contactPoints[contactPointCount++] = glm::vec4(projPointOntoRefPlane, 0.0f);
				}
				else if (startSignedDist < -0.0001f && endSignedDist > 0.0001f)
				{
					lerpRatio = startSignedDist / (startSignedDist - endSignedDist);
					lerpIntersectPoint = glm::mix(startVert, endVert, lerpRatio);

					if (getSignedDist(lerpIntersectPoint, referencePlane) < -0.0001f)
					{
						projPointOntoRefPlane = projectPointOntoPlane(lerpIntersectPoint, referencePlane);
						manifold.contactPoints[contactPointCount++] = glm::vec4(projPointOntoRefPlane, 0.0f);
					}
				}
			}

			startVert = endVert;
		}
	}

	manifold.contactBoxIndicesAndContactCount.x = referenceBoxIdx;
	manifold.contactBoxIndicesAndContactCount.y = incidentBoxIdx;
	manifold.contactBoxIndicesAndContactCount.z = contactPointCount;
	manifold.contactNormal = glm::vec4(referenceNormal, incidentLargestDist);

	return manifold;
}

Manifold createEdgeContact( EdgeQuery edgeQuery,
							BoxColliderGpuPackage const &boxColliderPackage, int boxAIdx, int boxBIdx)
{
	Manifold manifold;

	float s = 0, t = 0;

	glm::vec3 r = edgeQuery.pointsA[0] - edgeQuery.pointsB[0];
	float squaredLengthEdgeA = glm::dot(edgeQuery.edgeDirA, edgeQuery.edgeDirA); // a
	float squaredLengthEdgeB = glm::dot(edgeQuery.edgeDirB, edgeQuery.edgeDirB); // e
	float f = glm::dot(edgeQuery.edgeDirB, r);

	float c = dot(edgeQuery.edgeDirA, r);
	float b = dot(edgeQuery.edgeDirA, edgeQuery.edgeDirB);
	float denom = squaredLengthEdgeA * squaredLengthEdgeB - b * b; // a * e - b * b

	if (denom != 0.0f)
		s = glm::clamp((b * f) / denom, 0.0f, 1.0f); // Clamp to [0, 1]

	t = (b * s + f) / squaredLengthEdgeB;

	// For t, the clamping is a bit more complicated. We need to recalculate s if t is clamped.
	if (t < 0.0f)
	{
		t = 0.0f;
		s = glm::clamp(-c / squaredLengthEdgeA, 0.0f, 1.0f);
	}
	else if (t > 1.0f)
	{
		t = 1.0f;
		s = glm::clamp((b - c) / squaredLengthEdgeA, 0.0f, 1.0f);
	}

	glm::vec3 closestPointA = edgeQuery.pointsA[0] + s * edgeQuery.edgeDirA;
	glm::vec3 closestPointB = edgeQuery.pointsB[0] + t * edgeQuery.edgeDirB;

	// Choose the point in the middle of the two closest points above as the edge contact point
	manifold.contactPoints[0] = glm::vec4(0.5f * (closestPointB + closestPointA), 0.0f);

	// Set the collider pair indices, contact count, and the contact normal in the manifold
	manifold.contactBoxIndicesAndContactCount.x = boxAIdx;
	manifold.contactBoxIndicesAndContactCount.y = boxBIdx;
	manifold.contactBoxIndicesAndContactCount.z = 1;
	manifold.contactNormal = glm::vec4(edgeQuery.edgeNormal, edgeQuery.largestDist);

	return manifold;
}

// The size of the collisionPairs buffer can be sent here for a more elegant solution.
int P3Sat( BoxColliderGpuPackage const &boxColliders,
		   CollisionPairGpuPackage const &collisionPairs,
		   ManifoldGpuPackage const &manifolds )
{
	while (1)
	{
		break;
	}
	return 0;
}
