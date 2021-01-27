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

struct ManifoldGpuPackage
{
	Manifold manifolds[cMaxColliderCount]{};
};

#endif // P3_NARROW_PHASE_COMMON_H
