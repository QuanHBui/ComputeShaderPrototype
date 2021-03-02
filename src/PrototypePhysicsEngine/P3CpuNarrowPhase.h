/**
 * 3D Triangle-triangle intersection test
 *  There are 2 special cases to worry about: (1) Degenerate tri input, (2) coplanar tri-tri
 *
 * @author: Quan Bui
 * @version: 04/28/2020
 * @reference: Tomas Moller, "A Fast Triangle-Triangle Intersection Test"
 *			   https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/pubs/tritri.pdf
 */

#pragma once

#ifndef P3_CPU_NARROW_PHASE_H
#define P3_CPU_NARROW_PHASE_H

#include <glm/glm.hpp>

#include "P3NarrowPhaseCommon.h"

struct BoxColliderGpuPackage;
struct CollisionPairGpuPackage;
struct ManifoldGpuPackage;

bool coplanarTriTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &,
						glm::vec3 const &, glm::vec3 const &, glm::vec3 const &,
						glm::vec3 const &);

bool fastTriTriIntersect3DTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &,
							   glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);

void computeIntersectInterval(float, float, float,
							  float, float, float,
							  float, float,
							  float &, float &, bool &);

bool edgeEdgeTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
bool edgeTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
bool pointInTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);


namespace P3
{
class CpuNarrowPhase
{
public:
	void init() { mpManifoldPkg = new ManifoldGpuPackage(); }

	ManifoldGpuPackage *step(BoxColliderGpuPackage const &, const CollisionPairGpuPackage *);

	ManifoldGpuPackage const *getPManifoldPkg() const { return mpManifoldPkg; }

	~CpuNarrowPhase() { delete mpManifoldPkg; }

private:
	ManifoldGpuPackage *mpManifoldPkg;
};
}

#endif // P3_CPU_NARROW_PHASE_H
