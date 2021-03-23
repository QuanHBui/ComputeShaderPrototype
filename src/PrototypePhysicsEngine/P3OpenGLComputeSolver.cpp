#include "P3OpenGLComputeSolver.h"

#include "ComputeProgram.h"
#include "P3NarrowPhaseCommon.h"
#include "OpenGLUtils.h"

constexpr int cIterationCount = 5;

void P3OpenGLComputeSolver::init(GLuint manifoldBuffersID)
{
	mManifoldsID = manifoldBuffersID;

	GLbitfield mapFlags = GL_MAP_READ_BIT
						| GL_MAP_WRITE_BIT
						| GL_MAP_PERSISTENT_BIT
						| GL_MAP_COHERENT_BIT;

	GLbitfield createFlags = mapFlags | GL_DYNAMIC_STORAGE_BIT;

	glGenBuffers(4, mTransformBufferIDs);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mTransformBufferIDs[0]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(LinearTransform) * cMaxObjectCount, nullptr, createFlags);
	mpRigidLinearTransforms = static_cast<LinearTransform *>(glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(LinearTransform) * 512,
		mapFlags
	));
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mTransformBufferIDs[1]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(AngularTransform) * cMaxObjectCount, nullptr, createFlags);
	mpRigidAngularTransforms = static_cast<AngularTransform *>(glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(AngularTransform) * 512,
		mapFlags
	));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mTransformBufferIDs[2]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(LinearTransform) * cMaxObjectCount, nullptr, createFlags);
	mpStaticLinearTransforms = static_cast<LinearTransform *>(glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(LinearTransform) * 512,
		mapFlags
	));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mTransformBufferIDs[3]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(AngularTransform) * cMaxObjectCount, nullptr, createFlags);
	mpStaticAngularTransforms = static_cast<AngularTransform *>(glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(AngularTransform) * 512,
		mapFlags
	));

	mSolverComputeID = createComputeProgram("../resources/shaders/solver.comp");

	mDtUniformLoc = glGetUniformLocation(mSolverComputeID, "dt");
	mManifoldIdxUniformLoc = glGetUniformLocation(mSolverComputeID, "manifoldIdx");
	mRigidLinearSizeUniformLoc   = glGetUniformLocation(mSolverComputeID, "rigidLinearSize");
	mRigidAngularSizeUniformLoc  = glGetUniformLocation(mSolverComputeID, "rigidAngularSize");
	mStaticLinearSizeUniformLoc  = glGetUniformLocation(mSolverComputeID, "staticLinearSize");
	mStaticAngularSizeUniformLoc = glGetUniformLocation(mSolverComputeID, "staticAngularSize");

	mPreStepSubroutineIdx = glGetSubroutineIndex(mSolverComputeID, GL_COMPUTE_SHADER, "preStep");
	mIterativeSolveSubroutineIdx = glGetSubroutineIndex(mSolverComputeID, GL_COMPUTE_SHADER, "iterativeSolve");
};

void P3OpenGLComputeSolver::step( int manifoldPkgSize,
								  std::vector<LinearTransform> &rigidLinearTransformContainer,
								  std::vector<AngularTransform> &rigidAngularTransformContainer,
								  std::vector<LinearTransform> &staticLinearTransformContainer,
								  std::vector<AngularTransform> &staticAngularTransformContainer,
								  float dt)
{
	glUseProgram(mSolverComputeID);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mTransformBufferIDs[0]);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(LinearTransform) * rigidLinearTransformContainer.size(),
		rigidLinearTransformContainer.data()
	);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mTransformBufferIDs[1]);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(AngularTransform) * rigidAngularTransformContainer.size(),
		rigidAngularTransformContainer.data()
	);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mTransformBufferIDs[2]);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(LinearTransform) * staticLinearTransformContainer.size(),
		staticLinearTransformContainer.data()
	);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mTransformBufferIDs[3]);
	glBufferSubData(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(AngularTransform) * staticAngularTransformContainer.size(),
		staticAngularTransformContainer.data()
	);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mManifoldsID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mTransformBufferIDs[0]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mTransformBufferIDs[1]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mTransformBufferIDs[2]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mTransformBufferIDs[3]);

	glUniform1ui(mRigidLinearSizeUniformLoc, rigidLinearTransformContainer.size());
	glUniform1ui(mRigidAngularSizeUniformLoc, rigidAngularTransformContainer.size());
	glUniform1ui(mStaticLinearSizeUniformLoc, staticLinearTransformContainer.size());
	glUniform1ui(mStaticAngularSizeUniformLoc, staticAngularTransformContainer.size());

	glUniform1f(mDtUniformLoc, dt);

	// Prestep
	for (int i = 0; i < manifoldPkgSize; ++i)
	{
		glUniform1ui(mManifoldIdxUniformLoc, i);
		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &mPreStepSubroutineIdx);
		glDispatchCompute(1, 1, 1);
		glFlush();
	}

	for (int i = 0; i < cIterationCount; ++i)
	{
		for (int j = 0; j < manifoldPkgSize; ++j)
		{
			glUniform1ui(mManifoldIdxUniformLoc, j);
			glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &mIterativeSolveSubroutineIdx);
			glDispatchCompute(1, 1, 1);
			glFlush();
		}
	}

	for (int i = 0; i < rigidLinearTransformContainer.size(); ++i)
	{
		rigidLinearTransformContainer[i] = mpRigidLinearTransforms[i];
	}

	for (int j = 0; j < rigidAngularTransformContainer.size(); ++j)
	{
		rigidAngularTransformContainer[j] = mpRigidAngularTransforms[j];
	}

	for (int k = 0; k < staticLinearTransformContainer.size(); ++k)
	{
		staticLinearTransformContainer[k] = mpStaticLinearTransforms[k];
	}

	for (int l = 0; l < staticAngularTransformContainer.size(); ++l)
	{
		staticAngularTransformContainer[l] = mpStaticAngularTransforms[l];
	}
}