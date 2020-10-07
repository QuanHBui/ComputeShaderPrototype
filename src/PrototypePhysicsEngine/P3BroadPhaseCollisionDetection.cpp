#include "P3BroadPhaseCollisionDetection.h"

#include "../ComputeProgram.h"

void P3CpuBroadPhase(Aabb *pAabbContainer, size_t size)
{
	for (size_t i = 0u; i < size; ++i)
	{
		Aabb bodyi = pAabbContainer[i];
		
		for (size_t j = i; j < size; ++j) 
		{
			Aabb bodyj = pAabbContainer[j];

			// Check if the body is a static body, the one with infinite mass, or inverse mass of 0

			// Create Arbiter

			// Look out for contact points
		}
	}
}

void P3OpenGLComputeBroadPhase::init()
{
	initGpuBuffers();
}

void P3OpenGLComputeBroadPhase::simulateTimestep(float dt)
{
	buildBvhTreeOnGpu();
	detectCollisionPairs();
}

/**
 * Maybe hard reset on everything 
 * 
 */
void P3OpenGLComputeBroadPhase::reset()
{
	resetAtomicCounter();
}

void P3OpenGLComputeBroadPhase::initShaderPrograms()
{
	mComputeProgramIDContainer[P3_ASSIGN_MORTON_CODES] = createComputeProgram("../resources/shaders/assignMortonCodes.comp");
	mComputeProgramIDContainer[P3_BUILD_PARALLEL_LINEAR_BVH] = createComputeProgram("../resources/shaders/buildParallelLinearBVH.comp");
	mComputeProgramIDContainer[P3_SORT_LEAF_NODES] = createComputeProgram("../resources/shaders/sortLeafNodes.comp");
	mComputeProgramIDContainer[P3_UPDATE_AABBS] = createComputeProgram("../resources/shaders/updateAABBs.comp");
	mComputeProgramIDContainer[P3_DETECT_PAIRS] = createComputeProgram("../resources/shaders/detectPairs.comp");
}

/**
 * @input: A buffer of AABBs of all the objects in the physics dynamics world. 2 ways to get this:
 *  (1) Calculate the AABB of the shape once when loaded from Shape class, and then multiply all the transform matrices.
 *  (2) Dispatch compute one shader invocation per object and calculate the AABB after all of its vertices are transformed.
 * Quick judgement is that step (2) prob has more unneeded calculations.
 */
void P3OpenGLComputeBroadPhase::initGpuBuffers()
{
	glGenBuffers(NUM_BROAD_PHASE_SSBO, mSsboIDContainer.data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_AABB]);
	// Suggest OpenGL that this buffer is a GL_DYNAMIC_COPY because we assume that the physics engine on GPU is 
	//  in charge of changing the AABBs and then use them for drawing
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Aabb) * mpCreateInfo->aabbBufferSize, mpCreateInfo->pAabbBuffer, GL_DYNAMIC_COPY);

	// The output buffer. This is supposed to store the list of all the potential collision pairs. Allocate size of max num bodies.
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_COLLISION_PAIRS]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(CollisionPair) * mpCreateInfo->aabbBufferSize, nullptr, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &mAtomicBufferID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void P3OpenGLComputeBroadPhase::buildBvhTreeOnGpu()
{

}

void P3OpenGLComputeBroadPhase::detectCollisionPairs()
{
	glUseProgram(mComputeProgramIDContainer[P3_DETECT_PAIRS]);

	resetAtomicCounter();

	// Set binding points
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mSsboIDContainer[P3_AABB]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mSsboIDContainer[P3_COLLISION_PAIRS]);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, mAtomicBufferID);

	GLuint uniformIndex = glGetUniformBlockIndex(mComputeProgramIDContainer[P3_DETECT_PAIRS], "numObjects");
	glUniform1ui(uniformIndex, mpCreateInfo->aabbBufferSize);

	glDispatchCompute(1u, 1u, 1u);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
}

void P3OpenGLComputeBroadPhase::resetAtomicCounter()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	GLuint* pMappedBufferMemory = (GLuint *)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	memset(pMappedBufferMemory, 0, sizeof(GLuint));
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}