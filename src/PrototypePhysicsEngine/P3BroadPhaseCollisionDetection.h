#pragma once

#ifndef P3_BROAD_PHASE_COLLISION_DETECTION_H
#define P3_BROAD_PHASE_COLLISION_DETECTION_H

#include <array>

#include "BoundingVolume.h"
#include "../ComputeProgram.h"

#define NUM_BROAD_PHASE_COMPUTE_PROGRAMS 5
#define NUM_BROAD_PHASE_SSBO 2

void P3CpuBroadPhase(Aabb *, size_t);

struct P3OpenGLComputeBroadPhaseCreateInfo
{
	Aabb *pAabbBuffer = nullptr;
	glm::vec4 *pTransformBuffer = nullptr;	// What the heck is this for??
	GLuint aabbBufferSize = 0;
	GLuint transformBufferSize = 0;
};

class P3OpenGLComputeBroadPhase
{
public:
	P3OpenGLComputeBroadPhase() {}
	P3OpenGLComputeBroadPhase(P3OpenGLComputeBroadPhaseCreateInfo* createInfo)
		: mpCreateInfo(createInfo)
	{
		assert(createInfo && "createInfo is a nullptr");
	}

	void setCreateInfoPointer(P3OpenGLComputeBroadPhaseCreateInfo* pCreateInfo)
	{
		mpCreateInfo = pCreateInfo;
	}

	void init();

	//--------------------- Main proccesses --------------------------//
	void step(float);
	//----------------------------------------------------------------//

	void reset();

	~P3OpenGLComputeBroadPhase() {}

private:
	void initShaderPrograms();
	void initGpuBuffers();

	void buildBvhTreeOnGpu();
	void detectCollisionPairs();

	void resetAtomicCounter();
	
	// Indicies of shader programs
	enum
	{
		P3_ASSIGN_MORTON_CODES = 0,
		P3_SORT_LEAF_NODES,
		P3_BUILD_PARALLEL_LINEAR_BVH,
		P3_UPDATE_AABBS,
		P3_DETECT_PAIRS
	};

	// Indicices of ssbo
	enum
	{
		P3_AABB = 0,
		P3_COLLISION_PAIRS
		//P3_PARALLEL_LINEAR_BVH
	};

	//----------------------------- OpenGL bookkeeping ----------------------------//
	std::array<GLuint, NUM_BROAD_PHASE_COMPUTE_PROGRAMS> mComputeProgramIDContainer{ { 0u } };
	std::array<GLuint, NUM_BROAD_PHASE_SSBO> mSsboIDContainer{ { 0u } };
	GLuint mAtomicBufferID = 0u;

	P3OpenGLComputeBroadPhaseCreateInfo *mpCreateInfo = nullptr;

	//--------------------------- For testing purposes ----------------------------//
	GLuint mAtomicCounterCpu = 0u;
};

#endif // P3_BROAD_PHASE_COLLISION_DETECTION_H