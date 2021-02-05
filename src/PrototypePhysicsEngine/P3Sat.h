#pragma once

#ifndef P3_SAT_H
#define P3_SAT_H

struct BoxColliderGpuPackage;
struct CollisionPairGpuPackage;
struct ManifoldGpuPackage;

/**
 * This is the implementation of Seperating Axis Test on the CPU.
 */
ManifoldGpuPackage P3Sat(BoxColliderGpuPackage const &, const CollisionPairGpuPackage *);

#endif // P3_SAT_H
