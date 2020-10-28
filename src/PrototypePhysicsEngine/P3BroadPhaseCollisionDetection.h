#pragma once

#ifndef P3_BROAD_PHASE_COLLISION_DETECTION_H
#define P3_BROAD_PHASE_COLLISION_DETECTION_H

#include <array>
#include <vector>

#include "BoundingVolume.h"
#include "ComputeProgram.h"
#include "P3Collider.h"

#define NUM_BROAD_PHASE_COMPUTE_PROGRAMS 5
#define NUM_BROAD_PHASE_SSBOS 3

struct Aabb;

void P3CpuBroadPhase(Aabb *, size_t);

class P3OpenGLComputeBroadPhase
{
public:
	void init();

	//--------------------- Main proccesses --------------------------//
	void step(std::vector<P3MeshCollider> const &);
	//----------------------------------------------------------------//

	void reset();

	~P3OpenGLComputeBroadPhase() {}

private:
	void initShaderPrograms();
	void initGpuBuffers();

	void buildBvhTreeOnGpu();
	void detectCollisionPairs(std::vector<P3MeshCollider> const &);

	void resetAtomicCounter();

	// Indicies of shader programs
	enum
	{
		P3_UPDATE_AABBS = 0,
		P3_DETECT_PAIRS,
		P3_ASSIGN_MORTON_CODES,
		P3_SORT_LEAF_NODES,
		P3_BUILD_PARALLEL_LINEAR_BVH
	};

	// Indicices of ssbo
	enum
	{
		P3_MESH_COLLIDERS = 0,
		P3_AABBS,
		P3_COLLISION_PAIRS
	};

	//----------------------------- OpenGL bookkeeping ----------------------------//
	std::array<GLuint, NUM_BROAD_PHASE_COMPUTE_PROGRAMS> mComputeProgramIDContainer{ { 0u } };
	std::array<GLuint, NUM_BROAD_PHASE_SSBOS> mSsboIDContainer{ { 0u } };
	GLuint mAtomicBufferID = 0u;

	GLuint mAtomicCounterCpu = 0u;
	AabbGpuPackage mAabbCpuData;
};

#endif // P3_BROAD_PHASE_COLLISION_DETECTION_H