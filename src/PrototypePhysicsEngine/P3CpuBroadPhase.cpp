#include "P3CpuBroadPhase.h"

#include <algorithm>
#include <limits>
#include <map>

#include "P3Common.h"

namespace P3
{
CollisionPairGpuPackage *CpuBroadPhase::step(std::vector<P3BoxCollider> const &boxColliderContainer)
{
	std::map<int, Aabb> aabbMap;

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

		// Store the results - maybe we don't have to recalculate every frame
		aabbMap[i] = Aabb(glm::vec4(minX, minY, minZ, i), glm::vec4(maxX, maxY, maxZ, i));
	}

	// Sorting in x - PROBLEM IS YOU CAN'T REALLY SORT A MAP, THERE ARE HACKY WAYS TO DO IT THOUGH.
	//std::sort(aabbMap.begin(), aabbMap.end(), [](std::pair<int, Aabb> const &a, std::pair<int, Aabb> const &b)
	//	{
	//		return a.second.mMinCoord.x < b.second.mMinCoord.x;
	//	}
	//);

	// Sweep
	int sweepXIdx = 0;
	for (auto i = aabbMap.begin(); i != aabbMap.end(); ++i)
	{
		for (auto j = aabbMap.begin(); j != aabbMap.end(); ++j)
		{
			if (i == j) continue;

			if (i->second.mMinCoord.x < j->second.mMaxCoord.x && i->second.mMaxCoord.x > j->second.mMinCoord.x)
			{
				// For sweep x axis, we use x and y comps of collisionPkg to store the 2 indices.
				(*mpCollisionPairPkg)[sweepXIdx].x = i->first;
				(*mpCollisionPairPkg)[sweepXIdx++].y = j->first;
			}
		}
	}

	// Sort in y
	//std::sort(aabbMap.begin(), aabbMap.end(), [](Aabb const &a, Aabb const &b)
	//	{
	//		return a.mMinCoord.y < b.mMinCoord.y;
	//	}
	//);

	// Sweep
	int sweepYIdx = 0;
	for (int collisionPairIdx = 0; collisionPairIdx < sweepXIdx; ++collisionPairIdx)
	{
		Aabb aabb_1 = aabbMap[(*mpCollisionPairPkg)[collisionPairIdx].x];
		Aabb aabb_2 = aabbMap[(*mpCollisionPairPkg)[collisionPairIdx].y];

		if (aabb_1.mMinCoord.y < aabb_2.mMaxCoord.y && aabb_1.mMaxCoord.y > aabb_2.mMinCoord.y)
		{
			// For sweep y axis, we use z and w comps of collisionPkg to store the 2 indices.
			(*mpCollisionPairPkg)[sweepYIdx].z = int(aabb_1.mMinCoord.w); // w comp stores object ID
			(*mpCollisionPairPkg)[sweepYIdx++].w = int(aabb_2.mMinCoord.w);
		}
	}

	// Sort in z
	//std::sort(aabbMap.begin(), aabbMap.end(), [](Aabb const &a, Aabb const &b)
	//	{
	//		return a.mMinCoord.z < b.mMinCoord.z;
	//	}
	//);

	// Sweep
	int sweepZIdx = 0;
	for (int collisionPairIdx = 0; collisionPairIdx < sweepYIdx; ++collisionPairIdx)
	{
		Aabb aabb_1 = aabbMap[(*mpCollisionPairPkg)[collisionPairIdx].z];
		Aabb aabb_2 = aabbMap[(*mpCollisionPairPkg)[collisionPairIdx].w];

		if (aabb_1.mMinCoord.z < aabb_2.mMaxCoord.z && aabb_1.mMaxCoord.z > aabb_2.mMinCoord.z)
		{
			// Finally, for sweep z axis, we back use x and y comps of collisionPkg to store the 2 indices.
			// Also sort ID for consistency
			if (aabb_1.mMinCoord.w < aabb_2.mMinCoord.w)
			{
				(*mpCollisionPairPkg)[sweepZIdx].x = int(aabb_2.mMinCoord.w); // w comp stores object ID
				(*mpCollisionPairPkg)[sweepZIdx++].y = int(aabb_1.mMinCoord.w);
			}
			else
			{
				(*mpCollisionPairPkg)[sweepZIdx].x = int(aabb_1.mMinCoord.w); // w comp stores object ID
				(*mpCollisionPairPkg)[sweepZIdx++].y = int(aabb_2.mMinCoord.w);
			}
		}
	}

	mpCollisionPairPkg->misc.x = sweepZIdx;

	return mpCollisionPairPkg;
}
}