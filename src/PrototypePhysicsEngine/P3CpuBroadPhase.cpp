#include "P3CpuBroadPhase.h"

#include <algorithm>
#include <limits>

#include "P3Common.h"

namespace P3
{
CollisionPairGpuPackage *CpuBroadPhase::step(std::vector<P3BoxCollider> const &boxColliderContainer)
{
	std::vector<Aabb> aabbContainer;

	// Update Aabbs
	for (int i = 0; i < boxColliderContainer.size(); ++i)
	{
		P3BoxCollider const &boxCollider = boxColliderContainer[i];

		float minX, minY, minZ;
		float maxX, maxY, maxZ;

		minX = minY = minZ = std::numeric_limits<float>::max();
		maxX = maxY = maxZ = std::numeric_limits<float>::lowest();

		// Go through all vertices of each box collider
		for (int j = 0; j < cBoxColliderVertCount; ++j)
		{
			minX = std::min(minX, boxCollider[j].x);
			maxX = std::max(maxX, boxCollider[j].x);

			minY = std::min(minY, boxCollider[j].y);
			maxY = std::max(maxY, boxCollider[j].y);

			minZ = std::min(minZ, boxCollider[j].z);
			maxZ = std::max(maxZ, boxCollider[j].z);
		}

		// Store the results
		aabbContainer.emplace_back(glm::vec4(minX, minY, minZ, i), glm::vec4(maxX, maxY, maxZ, i));
	}

	// Sorting in x
	std::sort(aabbContainer.begin(), aabbContainer.end(), [](Aabb const &a, Aabb const &b)
		{
			return a.mMinCoord.x < b.mMinCoord.x;
		}
	);

	// Sweep
	int sweepXIdx = 0;
	for (int i = 0; i < aabbContainer.size(); ++i)
	{
		for (int j = 0; j < aabbContainer.size(); ++j)
		{
			if (i == j) continue;

			if (   aabbContainer[i].mMinCoord.x < aabbContainer[j].mMaxCoord.x
				&& aabbContainer[i].mMaxCoord.x > aabbContainer[j].mMinCoord.x )
			{
				// For sweep x axis, we use x and y comps of collisionPkg to store the 2 indices.
				(*mpCollisionPkg)[sweepXIdx++].x = int(aabbContainer[i].mMinCoord.w); // w comp stores object ID
				(*mpCollisionPkg)[sweepXIdx++].y = int(aabbContainer[j].mMinCoord.w);
			}
		}
	}


	// Sort in y
	std::sort(aabbContainer.begin(), aabbContainer.end(), [](Aabb const &a, Aabb const &b)
		{
			return a.mMinCoord.y < b.mMinCoord.y;
		}
	);

	// Sweep
	int sweepYIdx = 0;
	for (int i = 0; i < aabbContainer.size(); ++i)
	{
		for (int j = 0; j < aabbContainer.size(); ++j)
		{
			if (i == j) continue;

			for (int collisionPairIdx = 0; collisionPairIdx < sweepXIdx; ++collisionPairIdx)
			{
				if ((  (int(aabbContainer[i].mMinCoord.w) == (*mpCollisionPkg)[collisionPairIdx].x && int(aabbContainer[j].mMinCoord.w) == (*mpCollisionPkg)[collisionPairIdx].y)
					|| (int(aabbContainer[j].mMinCoord.w) == (*mpCollisionPkg)[collisionPairIdx].x && int(aabbContainer[i].mMinCoord.w) == (*mpCollisionPkg)[collisionPairIdx].y))
					&&  aabbContainer[i].mMinCoord.y < aabbContainer[j].mMaxCoord.y && aabbContainer[i].mMaxCoord.y > aabbContainer[j].mMinCoord.y)
				{
					// For sweep y axis, we use z and w comps of collisionPkg to store the 2 indices.
					(*mpCollisionPkg)[sweepYIdx++].z = int(aabbContainer[i].mMinCoord.w); // w comp stores object ID
					(*mpCollisionPkg)[sweepYIdx++].w = int(aabbContainer[j].mMinCoord.w);

					break;
				}
			}
		}
	}

	// Sort in z
	std::sort(aabbContainer.begin(), aabbContainer.end(), [](Aabb const &a, Aabb const &b)
		{
			return a.mMinCoord.z < b.mMinCoord.z;
		}
	);

	// Sweep
	int sweepZIdx = 0;
	for (int i = 0; i < aabbContainer.size(); ++i)
	{
		for (int j = 0; j < aabbContainer.size(); ++j)
		{
			if (i == j) continue;

			for (int collisionPairIdx = 0; collisionPairIdx < sweepXIdx; ++collisionPairIdx)
			{
				if ((  (int(aabbContainer[i].mMinCoord.w) == (*mpCollisionPkg)[collisionPairIdx].z && int(aabbContainer[j].mMinCoord.w) == (*mpCollisionPkg)[collisionPairIdx].w)
					|| (int(aabbContainer[j].mMinCoord.w) == (*mpCollisionPkg)[collisionPairIdx].z && int(aabbContainer[i].mMinCoord.w) == (*mpCollisionPkg)[collisionPairIdx].w))
					&&  aabbContainer[i].mMinCoord.z < aabbContainer[j].mMaxCoord.z && aabbContainer[i].mMaxCoord.z > aabbContainer[j].mMinCoord.z)
				{
					// Finally, for sweep z axis, we back use x and y comps of collisionPkg to store the 2 indices.
					// Also sort ID for consistency
					if (aabbContainer[i].mMinCoord.w < aabbContainer[j].mMinCoord.w)
					{
						(*mpCollisionPkg)[sweepZIdx++].x = int(aabbContainer[j].mMinCoord.w); // w comp stores object ID
						(*mpCollisionPkg)[sweepZIdx++].y = int(aabbContainer[i].mMinCoord.w);
					}
					else
					{
						(*mpCollisionPkg)[sweepZIdx++].x = int(aabbContainer[i].mMinCoord.w); // w comp stores object ID
						(*mpCollisionPkg)[sweepZIdx++].y = int(aabbContainer[j].mMinCoord.w);
					}
					break;
				}
			}
		}
	}

	mpCollisionPkg->misc.x = sweepZIdx;

	return mpCollisionPkg;
}
}