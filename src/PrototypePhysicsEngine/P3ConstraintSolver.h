#pragma once

#ifndef P3_CONSTRAINT_SOLVER
#define P3_CONSTRAINT_SOLVER

#include <vector>

#include <glad/glad.h>

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
	P3ConstraintSolver() {}

	// TODO: Overloaded constructor for any configuration setting
	P3ConstraintSolver(GLuint boxCollidersID, GLuint manifoldsID)
		: mBoxCollidersID(boxCollidersID), mManifoldsID(manifoldsID) {}

	// By default, reserve enough space for 10 things.
	void init(int reserveObjCount = 10)
	{
		for (int i = 0; i < reserveObjCount; ++i)
		{
			mLinearImpulseContainer.emplace_back();
			mAngularImpulseContainer.emplace_back();
		}
	};

	// If the solver's implementation resides on the CPU, returns the offset
	void solve( ManifoldGpuPackage const &,
				std::vector<P3BoxCollider> const &,
				std::vector<LinearTransform> const &,
				std::vector<AngularTransform> const &,
				std::vector<LinearTransform> const &,
				std::vector<AngularTransform> const & );
	// Else if on GPU, prob needs to know the handles of ManifoldGpuPackage from init()
	void solve();

	std::vector<glm::vec3> const &getLinearImpulseContainer() const { return mLinearImpulseContainer; }
	std::vector<glm::vec3> const &getAngularImpulseContainer() const { return mAngularImpulseContainer; }

	void reset();

	~P3ConstraintSolver() {}

private:
	void preStep(LinearTransform *, AngularTransform *, LinearTransform *, AngularTransform *);

	GLuint mBoxCollidersID = 0u, mManifoldsID = 0u;

	std::vector<glm::vec3> mLinearImpulseContainer;
	std::vector<glm::vec3> mAngularImpulseContainer;

	int mResetCounter = 0;
};

#endif // P3_CONSTRAINT_SOLVER