#pragma once

#ifndef P3_OPENGL_COMPUTE_SOLVER_H
#define P3_OPENGL_COMPUTE_SOLVER_H

#include <vector>

#include <glad/glad.h>

#include "P3Transform.h"

struct Manifold;
struct ManifoldGpuPackage;

class P3OpenGLComputeSolver
{
public:
	void init(GLuint);

	void step( int,
			   std::vector<LinearTransform> &,
			   std::vector<AngularTransform> &,
			   std::vector<LinearTransform> &,
			   std::vector<AngularTransform> &,
			   float );

private:
	GLuint mManifoldsID = 0u;
	GLuint mTransformBufferIDs[4];

	LinearTransform *mpRigidLinearTransforms;
	AngularTransform *mpRigidAngularTransforms;
	LinearTransform *mpStaticLinearTransforms;
	AngularTransform *mpStaticAngularTransforms;
	
	GLuint mDtUniformLoc = 0u;
	GLuint mManifoldIdxUniformLoc = 0u;
	GLuint mRigidLinearSizeUniformLoc   = 0u;
	GLuint mRigidAngularSizeUniformLoc  = 0u;
	GLuint mStaticLinearSizeUniformLoc  = 0u;
	GLuint mStaticAngularSizeUniformLoc = 0u;

	GLuint mPreStepSubroutineIdx = 0u;
	GLuint mIterativeSolveSubroutineIdx = 0u;

	GLuint mSolverComputeID = 0u;
};

#endif // P3_OPENGL_COMPUTE_SOLVER_H