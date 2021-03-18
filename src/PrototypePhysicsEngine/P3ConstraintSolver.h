#pragma once

#ifndef P3_CONSTRAINT_SOLVER
#define P3_CONSTRAINT_SOLVER

#include <vector>

#include "P3Common.h"

struct AngularTransform;
struct LinearTransform;
struct P3BoxCollider;
struct ManifoldGpuPackage;

/**
 * Take in a box collider list and a contact manifold list, then return an
 *  impulse list for resolution
 *
 * Much like the collision detection phase, this is also parallelized on the GPU
 *
 * Now, how do I make this configurable? Or what is there to configure?
 */
class P3ConstraintSolver
{
public:
	void preSolve( ManifoldGpuPackage &,
				   std::vector<LinearTransform> &,
				   std::vector<AngularTransform> &,
				   std::vector<LinearTransform> &,
				   std::vector<AngularTransform> &,
				   float );

	// Else if on GPU, prob needs to know the handles of ManifoldGpuPackage from init()
	void iterativeSolve( ManifoldGpuPackage &,
						 std::vector<LinearTransform> &,
						 std::vector<AngularTransform> &,
						 std::vector<LinearTransform> &,
						 std::vector<AngularTransform> & );

private:
	LinearTransform &getLinearTransform( int index,
										 std::vector<LinearTransform> &rigidLinearTransformContainer,
										 std::vector<LinearTransform> &staticLinearTransformContainer )
	{
		if (index >= rigidLinearTransformContainer.size())
		{
			return staticLinearTransformContainer[index - rigidLinearTransformContainer.size()];
		}
		else
		{
			return rigidLinearTransformContainer[index];
		}
	}

	AngularTransform &getAngularTransform( int index,
										   std::vector<AngularTransform> &rigidAngularTransformContainer,
										   std::vector<AngularTransform> &staticAngularTransformContainer )
	{
		if (index >= rigidAngularTransformContainer.size())
		{
			return staticAngularTransformContainer[index - rigidAngularTransformContainer.size()];
		}
		else
		{
			return rigidAngularTransformContainer[index];
		}
	}
};

#endif // P3_CONSTRAINT_SOLVER