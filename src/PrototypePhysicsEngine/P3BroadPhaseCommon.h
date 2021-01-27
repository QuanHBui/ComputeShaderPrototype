#pragma once

#ifndef P3_BROAD_PHASE_COMMON
#define P3_BROAD_PHASE_COMMON

#include "P3Common.h"

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

	glm::vec4 boxColliders[cMaxObjectCount][cBoxColliderVertCount]{};
};

struct CollisionPairGpuPackage
{
	glm::ivec4 collisionPairs[2 * cMaxObjectCount]{};
};

#endif // P3_BROAD_PHASE_COMMON
