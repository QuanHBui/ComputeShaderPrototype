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

class P3OpenGLComputeBroadPhase
{
public:
	void init();

	//--------------------- Main proccesses --------------------------//
	CollisionPairGpuPackage const &step(std::vector<P3BoxCollider> const &);
	//----------------------------------------------------------------//

	GLuint getBoxCollidersID() const { return mSsboIDContainer[P3_BOX_COLLIDERS]; };
	GLuint getCollisionPairsID() const { return mSsboIDContainer[P3_COLLISION_PAIRS]; }

	void reset();

	// TODO: Deallocate all Gpu buffers and delete programs.
	~P3OpenGLComputeBroadPhase() {}

private:
	void initShaderPrograms();
	void initGpuBuffers();

	void buildBvhTreeOnGpu();
	void detectCollisionPairs(std::vector<P3BoxCollider> const &);

	void resetAtomicCounter();
	GLuint readAtomicCounter();

	// Indicies of shader programs
	enum
	{
		P3_UPDATE_AABBS = 0,
		P3_ODD_EVEN_SORT,
		P3_SAP,
		P3_ASSIGN_MORTON_CODES,
		P3_SORT_LEAF_NODES,
		P3_BUILD_PARALLEL_LINEAR_BVH
	};

	// Indicices of ssbo's
	enum
	{
		P3_BOX_COLLIDERS = 0,
		P3_AABBS,
		P3_COLLISION_PAIRS
	};

	//----------------------------- OpenGL bookkeeping ----------------------------//
	std::array<GLuint, NUM_BROAD_PHASE_COMPUTE_PROGRAMS> mComputeProgramIDContainer{ 0u };
	std::array<GLuint, NUM_BROAD_PHASE_SSBOS> mSsboIDContainer{ 0u };
	GLuint mAtomicBufferID = 0u;

	//--------------------------------- Debug ---------------------------------//
	AabbGpuPackage mAabbCpuData;
	CollisionPairGpuPackage mCollisionPairCpuData;
};

#endif // P3_BROAD_PHASE_COLLISION_DETECTION_H
