#pragma once

#ifndef P3_OPENGL_COMPUTE_NARROW_PHASE_H
#define P3_OPENGL_COMPUTE_NARROW_PHASE_H

#include <unordered_map>
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
		mAtomicCounter.clear();
	}

private:
	void initShaderPrograms()
	{
		mComputeProgIDs[ComputeShader::SAT] = createComputeProgram("../resources/shaders/sat.comp");
	}

	void initGpuBuffers();

	enum class ComputeShader
	{
		SAT,
		TRI_TRI_TEST
	};

	enum class Buffer
	{
		GLOBAL,
		EXPERIMENTAL,
		BOX_COLLIDER,
		COLLISION_PAIR,
		MANIFOLD
	};

	GLuint mComputeProgramIDs[cNarrowPhaseComputeProgramCount]{};
	std::unordered_map<ComputeShader, GLuint> mComputeProgIDs{};
	std::unordered_map<Buffer, GLuint> mSsboIDs{};

	AtomicCounter mAtomicCounter;
	ManifoldGpuPackage *mpManifoldGpuPackage; // Stores the results from last physics tick
};

#endif // P3_OPENGL_COMPUTE_NARROW_PHASE_H
