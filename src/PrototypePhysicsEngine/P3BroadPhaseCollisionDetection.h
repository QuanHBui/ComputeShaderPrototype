#pragma once

#ifndef P3_BROAD_PHASE_COLLISION_DETECTION_H
#define P3_BROAD_PHASE_COLLISION_DETECTION_H

#include <array>

#include "RigidBody.h"
#include "ComputeProgram.h"

#define NUM_BROAD_PHASE_COMPUTE_PROGRAMS 5
#define NUM_BROAD_PHASE_SSBO 1

struct P3OpenGLComputeBroadPhaseCreateInfo
{
	glm::vec4 *pAabbPositionBuffer = nullptr;
	glm::vec4 *pTransformBuffer = nullptr;
	GLuint aabbPositionBufferSize = 0;
	GLuint transformBufferSize = 0;
};

class P3OpenGLComputeBroadPhase
{
public:
	P3OpenGLComputeBroadPhase(P3OpenGLComputeBroadPhaseCreateInfo *);

	void p3BuildBvhTreeOnGpu();
	void p3DetectCollisionPairs();

	~P3OpenGLComputeBroadPhase() {}

private:
	void initGpuBuffers();

	enum
	{
		P3_ASSIGN_MORTON_CODES = 0,
		P3_SORT_LEAF_NODES,
		P3_BUILD_PARALLEL_LINEAR_BVH,
		P3_UPDATE_AABBS,
		P3_DETECT_PAIRS
	};

	enum
	{
		P3_AABB_POSITIONS = 0,
		P3_PARALLEL_LINEAR_BVH
	};

	std::array<GLuint, NUM_BROAD_PHASE_COMPUTE_PROGRAMS> computeProgramContainer;
	std::array<GLuint, NUM_BROAD_PHASE_SSBO> mSsboContainer;

	P3OpenGLComputeBroadPhaseCreateInfo *mCreateInfo;
};

#endif // P3_BROAD_PHASE_COLLISION_DETECTION_H