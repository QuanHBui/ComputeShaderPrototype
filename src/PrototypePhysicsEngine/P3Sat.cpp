#include "P3Sat.h"

#include <array>
#include <limits>
#include <unordered_set>

#include <glm/gtx/hash.hpp>

#include "P3BroadPhaseCommon.h"
#include "P3NarrowPhaseCommon.h"

constexpr int cVertCountPerEdge  =  2;
constexpr int cVertCountPerFace  =  4;
constexpr int cColliderEdgeCount = 12;
constexpr int cColliderFaceCount =  6;
constexpr int cColliderVertCount =  8;
constexpr float cEpsilon = 0.0001f;

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
	float largestDist{ std::numeric_limits<float>::lowest() };
	glm::vec3 faceNormal{ 0.0f };
};

struct EdgeQuery
{
	float largestDist{ std::numeric_limits<float>::lowest() };
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
	return point - getSignedDist(point, plane) * plane.normal;
}

glm::vec3 getSupport(BoxCollider box, glm::vec3 const &direction)
{
	float projDist        = 0.0f;
	float largestProjDist = std::numeric_limits<float>::lowest();
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

glm::vec3 getFaceNormal(BoxCollider box, int faceIdx)
{
	glm::vec3 a{ box[cFaces[faceIdx][0]] };
	glm::vec3 b{ box[cFaces[faceIdx][1]] };
	glm::vec3 c{ box[cFaces[faceIdx][2]] };

	return glm::normalize(glm::cross(b - a, c - a));
}

Plane getPlane(BoxCollider const &box, int faceIdx)
{
	Plane plane;
	plane.point = box[cFaces[faceIdx][0]];
	plane.normal = getFaceNormal(box, faceIdx);

	return plane;
}

FaceQuery queryFaceDirections(BoxCollider boxA, BoxCollider boxB)
{
	float dist = 0.0f;
	Plane plane;
	glm::vec3 supportPoint{ 0.0f };
	FaceQuery faceQuery;

	for (int localFaceIdx = 0; localFaceIdx < cColliderFaceCount; ++localFaceIdx)
	{
		// Expand face of boxA to to a plane
		plane = getPlane(boxA, localFaceIdx);

		// Get support point from boxB
		supportPoint = getSupport(boxB, -plane.normal);

		// Get signed distance from support point to plane
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
	glm::vec3 temp{ 0.0f };
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
			endB   = boxB[cEdges[edgeIdxB][1]];
			edgeB  = endB - startB;

			// Make sure the 2 edges are not parallel
			temp = glm::cross(edgeA, edgeB);
			if (glm::length(temp) == 0.0f) continue;
			edgeNormal = glm::normalize(temp);

			if (glm::dot(edgeNormal, glm::normalize(startB - centerA)) < 0.0f)
				edgeNormal = -edgeNormal;

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

int getIncidentFaceIdx(BoxCollider incidentBox, glm::vec3 const &referenceNormal)
{
	int incidentFaceIdx = -1;
	glm::vec3 incidentNormal{ 0.0f };
	float smallestDotProduct = std::numeric_limits<float>::max();
	float localDotProduct    = 0.0f;

	for (int faceIdx = 0; faceIdx < cColliderFaceCount; ++faceIdx)
	{
		incidentNormal  = getFaceNormal(incidentBox, faceIdx);
		localDotProduct = glm::dot(incidentNormal, referenceNormal);

		if (localDotProduct < smallestDotProduct)
		{
			smallestDotProduct = localDotProduct;
			incidentFaceIdx    = faceIdx;
		}
	}

	return incidentFaceIdx;
}

// Filter out bad contact points - if this function is called, it's assumed that there are more than 4 contact points.
void reduceContactPoints(Manifold &manifold)
{
	// First contact point - query a support point in normal of contact plane
	int firstContactIdx     = -1;
	float largestSeparation = std::numeric_limits<float>::lowest();

	for (int i = 0; i < manifold.contactBoxIndicesAndContactCount.z; ++i)
	{
		float separation = glm::dot(glm::vec3(manifold.contacts[i].position), -glm::vec3(manifold.contactNormal));

		if (separation > largestSeparation)
		{
			largestSeparation = separation;
			firstContactIdx   = i;
		}
	}

	assert(firstContactIdx > -1);
	glm::vec3 a = manifold.contacts[firstContactIdx].position;

	// Second contact point - find the contact point farthest away from the first point
	int secondContactIdx = -1;
	largestSeparation    = std::numeric_limits<float>::lowest();

	for (int j = 0; j < manifold.contactBoxIndicesAndContactCount.z; ++j)
	{
		if (j == firstContactIdx) continue;

		float separation = glm::length(glm::vec3(manifold.contacts[j].position) - a);

		if (separation > largestSeparation)
		{
			largestSeparation = separation;
			secondContactIdx  = j;
		}
	}

	assert(secondContactIdx > -1);
	glm::vec3 b = manifold.contacts[secondContactIdx].position;

	// Third contact point - find the contact point that maximizes the triangle area
	int thirdContactIdx = -1;
	float largestArea   = std::numeric_limits<float>::lowest();

	for (int k = 0; k < manifold.contactBoxIndicesAndContactCount.z; ++k)
	{
		if (k == firstContactIdx || k == secondContactIdx) continue;

		glm::vec3 potC = manifold.contacts[k].position;
		float area = 0.5f * glm::dot(glm::cross(a - potC, b - potC), glm::vec3(manifold.contactNormal));

		if (area > largestArea)
		{
			largestArea = area;
			thirdContactIdx = k;
		}
	}

	assert(thirdContactIdx > -1);
	glm::vec3 c = manifold.contacts[thirdContactIdx].position;

	// Fourth contact point - find the contact point that maximizes the rectangle area
	int fourthContactIdx = -1;
	largestArea = std::numeric_limits<float>::max();

	for (int l = 0; l < manifold.contactBoxIndicesAndContactCount.z; ++l)
	{
		if (l == firstContactIdx || l == secondContactIdx || l == thirdContactIdx) continue;

		glm::vec3 potD = manifold.contacts[l].position;
		float area = 0.5f * glm::dot(glm::cross(a - potD, b - potD), glm::vec3(manifold.contactNormal));

		// Interested in most negative area, really depends on the winding of the triangle
		if (area < largestArea)
		{
			largestArea = area;
			fourthContactIdx = l;
		}
	}

	assert(fourthContactIdx > -1);
	glm::vec3 d = manifold.contacts[fourthContactIdx].position;

	// The w component could be anything really.
	manifold.contacts[0].position = glm::vec4(a, 1.0f);
	manifold.contacts[1].position = glm::vec4(b, 1.0f);
	manifold.contacts[2].position = glm::vec4(c, 1.0f);
	manifold.contacts[3].position = glm::vec4(d, 1.0f);
	manifold.contactBoxIndicesAndContactCount.z = 4;
}

Manifold createFaceContact( FaceQuery const &faceQueryA, FaceQuery const &faceQueryB,
							BoxCollider boxA, BoxCollider boxB,
							int boxAIdx, int boxBIdx )
{
	int referenceBoxIdx = -1;
	int incidentBoxIdx  = -1;
	int incidentFaceIdx = -1;
	float referenceSeparation = 0.0f;
	Plane referencePlane;
	BoxCollider incidentBox  = nullptr;
	BoxCollider referenceBox = nullptr;
	constexpr float cAxisBias = 0.4f;

	// Identify reference plane, then incident face
	// Apply a bias to prefer a certain axis of penetration, i.e the rigid body feature
	if (cAxisBias * faceQueryA.largestDist > faceQueryB.largestDist)
	{
		referencePlane  = getPlane(boxA, faceQueryA.faceIdx);
		referenceBoxIdx = boxAIdx;
		referenceBox    = boxA;
		referenceSeparation = faceQueryA.largestDist; // Does this make sense?

		incidentFaceIdx = getIncidentFaceIdx(boxB, referencePlane.normal);
		incidentBoxIdx  = boxBIdx;
		incidentBox     = boxB;
	}
	else
	{
		referencePlane  = getPlane(boxB, faceQueryB.faceIdx);
		referenceBoxIdx = boxBIdx;
		referenceBox    = boxB;
		referenceSeparation = faceQueryB.largestDist;

		incidentFaceIdx = getIncidentFaceIdx(boxA, referencePlane.normal);
		incidentBoxIdx  = boxAIdx;
		incidentBox     = boxA;
	}

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

	// TODO: Need some sort of way to keep track of what points already got clipped out. If it already got clipped
	//  by a plane, then it wouldn't be considered to be clipped again.
	// Also, there might be duplicates, i.e store the vert that's already stored.
	std::unordered_set<glm::vec3> includedVertSet; // Oh yea, this is big brain time.
	std::unordered_set<glm::vec3> clippedVertSet;
	constexpr int actualIndices[4] = { 1, 2, 3, 0 };

	// Iterate over all the faces of the reference box
	for (int faceIdx = 0; faceIdx < cColliderFaceCount; ++faceIdx)
	{
		clipPlane = getPlane(referenceBox, faceIdx);
		if (glm::abs(glm::dot(referencePlane.normal, clipPlane.normal)) <= cEpsilon)
		{
			// Start from vert# 0 of the incident face - since there's only 1 incident face, we can cache some data regarding it.
			startVertIdx = cFaces[incidentFaceIdx][0];
			startVert    = incidentBox[startVertIdx];

			// Iterate through vert# 1, 2, 3, 0 of the incident face
			for (int vertIdx = 0; vertIdx < cVertCountPerFace; ++vertIdx)
			{
				endVertIdx = cFaces[incidentFaceIdx][actualIndices[vertIdx]];
				endVert = incidentBox[endVertIdx];

				startSignedDist = getSignedDist(startVert, clipPlane);
				endSignedDist   = getSignedDist(endVert, clipPlane);

				// If start vert is on the positive side, store the end vert and lerp the intersection. Start vert is clipped.
				if (startSignedDist >= cEpsilon && endSignedDist < -cEpsilon)
				{
					// This might be a bad clipping logic, don't know what to do if the end vert not below the reference face.
					//  Does that get clipped too?
					// Start vertex idx here is just current vert idx - 1
					projPointOntoRefPlane = projectPointOntoPlane(startVert, referencePlane);
					clippedVertSet.insert(projPointOntoRefPlane);

					if (getSignedDist(endVert, referencePlane) <= -cEpsilon)
					{
						projPointOntoRefPlane = projectPointOntoPlane(endVert, referencePlane);
						includedVertSet.insert(projPointOntoRefPlane);
					}

					lerpRatio = startSignedDist / (startSignedDist - endSignedDist);
					lerpIntersectPoint = glm::mix(startVert, endVert, lerpRatio);

					if (getSignedDist(lerpIntersectPoint, referencePlane) <= -cEpsilon)
					{
						projPointOntoRefPlane = projectPointOntoPlane(lerpIntersectPoint, referencePlane);
						includedVertSet.insert(projPointOntoRefPlane);
					}
				}

				// Both start and end vertices are on the negative side, store only the end vert because start vert already got
				//  stored from the previous iteration.
				else if (   startSignedDist < -cEpsilon && endSignedDist < -cEpsilon
						 && getSignedDist(endVert, referencePlane) <= -cEpsilon )
				{
					projPointOntoRefPlane = projectPointOntoPlane(endVert, referencePlane);
					includedVertSet.insert(projPointOntoRefPlane);
				}

				// If start vert is on the negative side, and end vert is on the positive side, only lerp the intersection,
				//  then clip the end vert.
				else if (startSignedDist < -cEpsilon && endSignedDist >= cEpsilon)
				{
					projPointOntoRefPlane = projectPointOntoPlane(endVert, referencePlane);
					clippedVertSet.insert(projPointOntoRefPlane);

					lerpRatio = startSignedDist / (startSignedDist - endSignedDist);
					lerpIntersectPoint = glm::mix(startVert, endVert, lerpRatio);

					if (getSignedDist(lerpIntersectPoint, referencePlane) <= -cEpsilon)
					{
						projPointOntoRefPlane = projectPointOntoPlane(lerpIntersectPoint, referencePlane);
						includedVertSet.insert(projPointOntoRefPlane);
					}
				}

				startVert = endVert;
			}
		}
	}

	int contactPointCount = 0;
	Manifold manifold;

	// Process the clipped and included sets. Once the vert got clipped, game over.
	// Iterate through the included set, check if it's got clip in the clipped set; if not, store it as contact point
	for (glm::vec3 const &potContactPoint : includedVertSet)
	{
		if (contactPointCount < cMaxContactPointCount && clippedVertSet.find(potContactPoint) == clippedVertSet.end())
		{
			manifold.contacts[contactPointCount++].position = glm::vec4(potContactPoint, 1.0f);
		}
	}

	manifold.contactBoxIndicesAndContactCount.x = referenceBoxIdx;
	manifold.contactBoxIndicesAndContactCount.y = incidentBoxIdx;
	manifold.contactBoxIndicesAndContactCount.z = contactPointCount;
	manifold.contactNormal = glm::vec4(referencePlane.normal, referenceSeparation);

	if (contactPointCount > 4)
	{
		reduceContactPoints(manifold);
	}

	return manifold;
}

Manifold createEdgeContact(EdgeQuery edgeQuery, int boxAIdx, int boxBIdx)
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
		s = glm::clamp((b * f - c * squaredLengthEdgeB) / denom, 0.0f, 1.0f); // Clamp to [0, 1]

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
	manifold.contacts[0].position = glm::vec4(0.5f * (closestPointB + closestPointA), 0.0f);

	// Set the collider pair indices, contact count, and the contact normal in the manifold
	manifold.contactBoxIndicesAndContactCount.x = boxAIdx;
	manifold.contactBoxIndicesAndContactCount.y = boxBIdx;
	manifold.contactBoxIndicesAndContactCount.z = 1;
	manifold.contactNormal = glm::vec4(edgeQuery.edgeNormal, edgeQuery.largestDist);

	return manifold;
}

// The size of the collisionPairs buffer can be sent here for a more elegant solution.
void P3::sat( ManifoldGpuPackage *pManifoldPkg,
			  BoxColliderGpuPackage const &boxColliderPkg,
			  const CollisionPairGpuPackage *pCollisionPairPkg )
{
	int availableIdx = 0;
	int boxAIdx = -1;
	int boxBIdx = -1;
	BoxCollider boxA = nullptr;
	BoxCollider boxB = nullptr;
	constexpr float cQueryBias = 0.5f;

	for (int collisionPairIdx = 0; collisionPairIdx < pCollisionPairPkg->misc.x; ++collisionPairIdx)
	{
		boxAIdx = pCollisionPairPkg->collisionPairs[collisionPairIdx].x;
		boxBIdx = pCollisionPairPkg->collisionPairs[collisionPairIdx].y;
		boxA = boxColliderPkg[boxAIdx];
		boxB = boxColliderPkg[boxBIdx];

		// Look at faces of A
		FaceQuery faceQueryA = queryFaceDirections(boxA, boxB);
		if (faceQueryA.largestDist > 0.0f) continue; // We have found a separating axis. No overlap.

		FaceQuery faceQueryB = queryFaceDirections(boxB, boxA); // Look at faces of B
		if (faceQueryB.largestDist > 0.0f) continue;

		//EdgeQuery edgeQuery = queryEdgeDirections(boxA, boxB); // Look at edges of A and B
		//// TODO: This is stupidly hacky, don't leave this like this.
		//if (edgeQuery.largestDist > 0) edgeQuery.largestDist = -edgeQuery.largestDist;

		//if (edgeQuery.largestDist > 0.0f) continue;

		// If we get to here, there's no separating axis, the 2 boxes must overlap.
		// Remember that at this point, largestFaceADist, largestFaceBDist, and edgeLargestDist
		//  are all negative, so whichever is the least negative is the minimum penetration distance.
		// Find the closest feature type
		Manifold manifold;

		// Apply a bias to prefer face contact over edge contact in the case when the separations returned
		//  from the face query and edge query are the same.
		//if (   cQueryBias * faceQueryA.largestDist > edgeQuery.largestDist
		//	&& cQueryBias * faceQueryB.largestDist > edgeQuery.largestDist )
		//{
		//	manifold = createFaceContact(faceQueryA, faceQueryB, boxA, boxB, boxAIdx, boxBIdx);
		//}
		//else
		//{
		//	manifold = createEdgeContact(edgeQuery, boxAIdx, boxBIdx);
		//}
		manifold = createFaceContact(faceQueryA, faceQueryB, boxA, boxB, boxAIdx, boxBIdx);

		pManifoldPkg->manifolds[availableIdx++] = manifold;
	}

	pManifoldPkg->misc.x = availableIdx;
}
