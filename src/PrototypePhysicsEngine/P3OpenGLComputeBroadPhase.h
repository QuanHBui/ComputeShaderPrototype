#pragma once

#ifndef P3_OPENGL_COMPUTE_BROAD_PHASE_H
#define P3_OPENGL_COMPUTE_BROAD_PHASE_H

#include <array>
#include <vector>

#include "AtomicCounter.h"
#include "BoundingVolume.h"
#include "ComputeProgram.h"
#include "P3Collider.h"
#include "P3BroadPhaseCommon.h"

constexpr int cBroadPhaseComputeProgramCount = 5;
constexpr GLsizei cBroadPhaseSsboCount = 3;

class P3OpenGLComputeBroadPhase
{
public:
	void init();

	//--------------------- Main proccesses --------------------------//
	CollisionPairGpuPackage const &step(std::vector<P3BoxCollider> const &);
	//----------------------------------------------------------------//

	GLuint getBoxCollidersID() const { return mSsboIDs[P3_BOX_COLLIDERS]; };
	GLuint getCollisionPairsID() const { return mSsboIDs[P3_COLLISION_PAIRS]; }

	void reset();

	// TODO: Deallocate all Gpu buffers and delete programs.
	~P3OpenGLComputeBroadPhase() {}

private:
	void initShaderPrograms();
	GLuint initGpuBuffers();

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
	std::array<GLuint, cBroadPhaseComputeProgramCount> mComputeProgramIDContainer{};
	std::array<GLuint, cBroadPhaseSsboCount> mSsboIDs{};

	GLuint mDispatchIndirectBufferID = 0;
	DispatchIndirectCommand mDispatchIndirectCommand{ 1, 1, 1 };

	//--------------------------------- CPU data ---------------------------------//
	AtomicCounter mAtomicCounter;
	AabbGpuPackage mAabbCpuData;
	BoxColliderGpuPackage *mpBoxColliderCpuData = nullptr;  // Data streaming to GPU
	CollisionPairGpuPackage *mpCollisionPairCpuData = nullptr;
};

#endif // P3_OPENGL_COMPUTE_BROAD_PHASE_H
