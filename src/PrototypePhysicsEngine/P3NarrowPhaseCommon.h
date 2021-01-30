#pragma once

#ifndef P3_NARROW_PHASE_COMMON_H
#define P3_NARROW_PHASE_COMMON_H

#include "P3Common.h"

struct Manifold
{
	glm::ivec4 contactBoxIndicesAndContactCount{}; // x = refBoxIdx, y = incidentBoxIdx, z = contact count
	glm::vec4 contactPoints[cMaxContactPointCount]{};
	glm::vec4 contactNormal{}; // w stores the penetration depth.
};

struct ManifoldGpuPackage // To be replaced by the struct below
{
	glm::ivec4 misc{};
	Manifold manifolds[cMaxColliderCount]{};
};

// Let's make this more data driven, meaning it doesn't make any physical sense but it's easy to move/map data around.
struct ManifoldPackage
{
	glm::ivec4 misc{}; // Must use ivec4 for appropriate memory padding, real data start at offset of 16 bytes
	glm::ivec4 contactBoxIndicesAndContactCount[cMaxColliderCount]{};
	glm::vec4 contactPoints[cMaxContactPointCount][cMaxColliderCount]{};
	glm::vec4 contactNormal[cMaxColliderCount]{};
};

#endif // P3_NARROW_PHASE_COMMON_H
