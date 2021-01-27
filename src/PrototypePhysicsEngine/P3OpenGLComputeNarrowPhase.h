#pragma once

#ifndef P3_OPENGL_COMPUTE_NARROW_PHASE_H
#define P3_OPENGL_COMPUTE_NARROW_PHASE_H

#include <glad/glad.h>

#include "AtomicCounter.h"
#include "ComputeProgram.h"
#include "P3Common.h"

constexpr uint16_t cNarrowPhaseComputeProgramCount = 1u;
constexpr GLsizei cNarrowPhaseSsboCount = 1u;

struct BoundingVolume;
struct ManifoldGpuPackage;

class P3OpenGLComputeNarrowPhase
{
public:
	void init(GLuint, GLuint);

	ManifoldGpuPackage const &step(uint16_t);
	
	void reset();

	~P3OpenGLComputeNarrowPhase()
	{
		for (uint16_t i = 0u; i < cNarrowPhaseComputeProgramCount; ++i)
			glDeleteProgram(mComputeProgramIDs[i]);

		glDeleteBuffers(cNarrowPhaseSsboCount, mSsboIDs);

		mAtomicCounter.clear();
	}

private:
	void initShaderPrograms()
	{
		mComputeProgramIDs[SAT] = createComputeProgram("../resources/shaders/sat.comp");
	}

	void initGpuBuffers();

	enum ComputeShader : uint8_t
	{
		SAT = 0,
		TRI_TRI_TEST
	};

	enum StorageShaderBuffer : uint8_t
	{
		MANIFOLDS = 0,
		MTVS
	};

	// Some Gpu buffers already created from the broad phase
	GLuint mBoxCollidersID = 0u, mCollisionPairsID = 0u;

	GLuint mComputeProgramIDs[cNarrowPhaseComputeProgramCount]{};
	GLuint mSsboIDs[cNarrowPhaseSsboCount]{};

	AtomicCounter mAtomicCounter;
	ManifoldGpuPackage *mpManifoldGpuPackage; // Stores the results from last physics tick
};

#endif // P3_OPENGL_COMPUTE_NARROW_PHASE_H
