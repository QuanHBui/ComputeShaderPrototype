#pragma once

#ifndef P3_OPENGL_COMPUTE_NARROW_PHASE_H
#define P3_OPENGL_COMPUTE_NARROW_PHASE_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "AtomicCounter.h"
#include "ComputeProgram.h"

constexpr int cMaxContactPointCount = 16;
constexpr int cMaxColliderCount = 1024;
constexpr uint16_t cNarrowPhaseComputeProgramCount = 1u;
constexpr GLsizei cNarrowPhaseSsboCount = 1u;

struct BoundingVolume;

struct Manifold
{
	glm::ivec4 contactBoxIndicesAndContactCount{}; // x = refBoxIdx, y = incidentBoxIdx, z = contact count
	glm::vec4 contactPoints[cMaxContactPointCount]{};
	glm::vec4 contactNormal{}; // w stores the penetration depth.
};

struct ManifoldGpuPackage
{
	Manifold manifolds[cMaxColliderCount]{};
};

void buildContactManifold(Manifold &, BoundingVolume *, BoundingVolume *);

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
