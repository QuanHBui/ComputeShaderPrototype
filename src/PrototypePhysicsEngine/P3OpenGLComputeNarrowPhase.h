#pragma once

#ifndef P3_OPENGL_COMPUTE_NARROW_PHASE_H
#define P3_OPENGL_COMPUTE_NARROW_PHASE_H

#include <unordered_map>
#include <glad/glad.h>

#include "AtomicCounter.h"
#include "ComputeProgram.h"
#include "P3Common.h"

constexpr uint16_t cNarrowPhaseComputeProgramCount = 1u;
constexpr GLsizei cNarrowPhaseSsboCount = 2u;

struct BoundingVolume;
struct ManifoldGpuPackage;

class P3OpenGLComputeNarrowPhase
{
public:
	void init(GLuint, GLuint);

	void step();

	ManifoldGpuPackage *getPManifoldPkg() { return mpManifoldPkg[mFrontBufferIdx]; }
	
	// Only call this if you know what you are doing
	void swapBuffers()
	{
		mFrontBufferIdx = !mFrontBufferIdx;

		GLuint temp = mSsboIDs[Buffer::MANIFOLD_FRONT];
		mSsboIDs[Buffer::MANIFOLD_FRONT] = mSsboIDs[Buffer::MANIFOLD_BACK];
		mSsboIDs[Buffer::MANIFOLD_BACK]  = temp;
	}

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
		BOX_COLLIDER,
		COLLISION_PAIR,
		MANIFOLD_FRONT,
		MANIFOLD_BACK
	};

	GLuint mComputeProgramIDs[cNarrowPhaseComputeProgramCount]{};
	std::unordered_map<ComputeShader, GLuint> mComputeProgIDs{};
	std::unordered_map<Buffer, GLuint> mSsboIDs{};

	AtomicCounter mAtomicCounter{};
	ManifoldGpuPackage *mpManifoldPkg[2]; // Stores the results from last physics tick

	int mFrontBufferIdx = 0;
};

#endif // P3_OPENGL_COMPUTE_NARROW_PHASE_H
