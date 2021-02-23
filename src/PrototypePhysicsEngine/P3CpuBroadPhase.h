#pragma once

#ifndef P3_CPU_BROAD_PHASE_H
#define P3_CPU_BROAD_PHASE_H

#include <vector>

#include "P3BroadPhaseCommon.h"
#include "P3Collider.h"

namespace P3
{
class CpuBroadPhase
{
public:
	void init() { mpCollisionPairPkg = new CollisionPairGpuPackage(); }
	CollisionPairGpuPackage *step(std::vector<P3BoxCollider> const &);

	CollisionPairGpuPackage const *getPCollisionPkg() const { return mpCollisionPairPkg; }

	~CpuBroadPhase() { delete mpCollisionPairPkg; }

private:
	CollisionPairGpuPackage *mpCollisionPairPkg;
};
}

#endif // P3_CPU_BROAD_PHASE_H
