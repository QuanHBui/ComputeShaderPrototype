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
	CpuBroadPhase() { /*mpCollisionPkg = new CollisionPairGpuPackage(); */}

	void init() { mpCollisionPkg = new CollisionPairGpuPackage(); }
	CollisionPairGpuPackage *step(std::vector<P3BoxCollider> const &);

	CollisionPairGpuPackage const *getPCollisionPkg() const { return mpCollisionPkg; }

	~CpuBroadPhase() { delete mpCollisionPkg; }

private:
	CollisionPairGpuPackage *mpCollisionPkg;
};
}

#endif // P3_CPU_BROAD_PHASE_H
