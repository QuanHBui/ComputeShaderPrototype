#pragma once

#ifndef P3_CONSTRAINT_SOLVER
#define P3_CONSTRAINT_SOLVER

#include <glad/glad.h>

#include "P3BroadPhaseCollisionDetection.h"

struct BoxColliderGpuPackage;
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
	P3ConstraintSolver() {}

	// TODO: Overloaded constructor for any configuration setting
	P3ConstraintSolver(GLuint boxCollidersID, GLuint manifoldsID)
		: mBoxCollidersID(boxCollidersID), mManifoldsID(manifoldsID) {}

	void init();

	// If the solver's implementation resides on the CPU
	void solve(BoxColliderGpuPackage const &, ManifoldGpuPackage const &);
	// Else if on GPU
	void solve();

	~P3ConstraintSolver() {}

private:
	GLuint mBoxCollidersID = 0u, mManifoldsID = 0u;
};

#endif // P3_CONSTRAINT_SOLVER