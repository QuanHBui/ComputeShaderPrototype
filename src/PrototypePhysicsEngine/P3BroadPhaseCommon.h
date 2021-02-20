#pragma once

#ifndef P3_BROAD_PHASE_COMMON
#define P3_BROAD_PHASE_COMMON

#include "P3Common.h"

struct Aabb
{
	Aabb() {};
	Aabb(glm::vec4 const &minCoord, glm::vec4 const &maxCoord)
		: mMinCoord(minCoord), mMaxCoord(maxCoord) {}

	glm::vec4 mMinCoord{};
	glm::vec4 mMaxCoord{};
};

//------------------ Data packs for the GPU (SoA) --------------------//
struct AabbGpuPackage
{
	glm::vec4 minCoords[cMaxObjectCount]{};
	glm::vec4 maxCoords[cMaxObjectCount]{};
};

// This potentially can store the size of the boxColliders, to solve the potential
//  padding issue, the size can be padded to 16 bytes, or just use a glm::vec4
struct BoxColliderGpuPackage
{
	glm::vec4 const *operator[](int boxIdx) const { return boxColliders[boxIdx]; }

	glm::ivec4 misc{};
	glm::vec4 boxColliders[cMaxObjectCount][cBoxColliderVertCount]{};
};

struct CollisionPairGpuPackage
{
	glm::ivec4 const &operator[](int boxIdx) const { return collisionPairs[boxIdx]; }
	glm::ivec4 &operator[](int boxIdx) { return collisionPairs[boxIdx]; }

	glm::ivec4 misc{};
	glm::ivec4 collisionPairs[2 * cMaxObjectCount]{};
};

#endif // P3_BROAD_PHASE_COMMON
