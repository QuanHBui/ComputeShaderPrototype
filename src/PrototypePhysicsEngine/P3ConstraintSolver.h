#pragma once

#ifndef P3_CONSTRAINT_SOLVER
#define P3_CONSTRAINT_SOLVER

#include <vector>

#include <glad/glad.h>

#include "P3Common.h"

struct LinearTransform;
struct AngularTransform;
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

	void init() {};

	// If the solver's implementation resides on the CPU, returns the offset
	int solve( ManifoldGpuPackage const &, std::vector<LinearTransform> const &,
			   std::vector<AngularTransform> const & );
	// Else if on GPU, prob needs to know the handles of ManifoldGpuPackage from init()
	void solve();

	std::vector<glm::vec3> const &getImpulseContainer() const { return mImpulseContainer; }

	~P3ConstraintSolver() {}

private:
	GLuint mBoxCollidersID = 0u, mManifoldsID = 0u;

	std::vector<glm::vec3> mImpulseContainer;
};

#endif // P3_CONSTRAINT_SOLVER