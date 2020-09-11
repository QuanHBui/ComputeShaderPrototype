#include "P3BroadPhaseCollisionDetection.h"

#include "ComputeProgram.h"

P3OpenGLComputeBroadPhase::P3OpenGLComputeBroadPhase(P3OpenGLComputeBroadPhaseCreateInfo *createInfo)
	: mCreateInfo(createInfo)
{
	assert(createInfo && "createInfo is a nullptr");

	computeProgramContainer[P3_ASSIGN_MORTON_CODES] = createComputeProgram("../resources/shaders/assignMortonCodes.comp");
	computeProgramContainer[P3_BUILD_PARALLEL_LINEAR_BVH] = createComputeProgram("../resources/shaders/buildParallelLinearBVH.comp");
	computeProgramContainer[P3_SORT_LEAF_NODES] = createComputeProgram("../resources/shaders/sortLeafNodes.comp");
	computeProgramContainer[P3_UPDATE_AABBS] = createComputeProgram("../resources/shaders/updateAABBs.comp");
	computeProgramContainer[P3_DETECT_PAIRS] = createComputeProgram("../resources/shaders/detectPairs.comp");
}

/**
 * @input: A buffer of AABBs of all the objects in the physics dynamics world. 2 ways to get this:
 *  (1) Calculate the AABB of the shape once when loaded from Shape class, and then multiply all the transform matrices.
 *  (2) Dispatch compute one shader invocation per object and calculate the AABB after all of its vertices are transformed.
 * Quick judgement is that step (2) prob has more unneeded calculations.
 */
void P3OpenGLComputeBroadPhase::initGpuBuffers()
{
	P3OpenGLComputeBroadPhaseCreateInfo createInfo = *mCreateInfo;

	glGenBuffers(NUM_BROAD_PHASE_SSBO, mSsboContainer.data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboContainer[P3_AABB_POSITIONS]);
	// Suggest OpenGL that this buffer is a GL_DYNAMIC_COPY because we assume that the physics engine on GPU is in charge of changing the AABBs and then use them for drawing
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * mCreateInfo->aabbPositionBufferSize, mCreateInfo->pAabbPositionBuffer, GL_DYNAMIC_COPY);
}