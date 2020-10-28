#include "P3BroadPhaseCollisionDetection.h"

#include "ComputeProgram.h"
#include "GLSL.h"

void P3CpuBroadPhase(Aabb *pAabbContainer, unsigned int size)
{
	for (unsigned int i = 0u; i < size; ++i)
	{
		Aabb bodyi = pAabbContainer[i];

		for (unsigned int j = i; j < size; ++j)
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
	initShaderPrograms();
	initGpuBuffers();
}

void P3OpenGLComputeBroadPhase::step(std::vector<P3MeshCollider> const &meshColliderContainer)
{
	detectCollisionPairs(meshColliderContainer);
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
	mComputeProgramIDContainer[P3_UPDATE_AABBS] = createComputeProgram("../resources/shaders/updateAABBs.comp");
	mComputeProgramIDContainer[P3_DETECT_PAIRS] = createComputeProgram("../resources/shaders/detectPairs.comp");
	mComputeProgramIDContainer[P3_ASSIGN_MORTON_CODES] = createComputeProgram("../resources/shaders/assignMortonCodes.comp");
	mComputeProgramIDContainer[P3_BUILD_PARALLEL_LINEAR_BVH] = createComputeProgram("../resources/shaders/buildParallelLinearBVH.comp");
	mComputeProgramIDContainer[P3_SORT_LEAF_NODES] = createComputeProgram("../resources/shaders/sortLeafNodes.comp");
}

void P3OpenGLComputeBroadPhase::initGpuBuffers()
{
	CHECKED_GL_CALL(glGenBuffers(NUM_BROAD_PHASE_SSBOS, mSsboIDContainer.data()));

	glGenBuffers(1, &mAtomicBufferID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void P3OpenGLComputeBroadPhase::buildBvhTreeOnGpu()
{

}

void P3OpenGLComputeBroadPhase::detectCollisionPairs(std::vector<P3MeshCollider> const &meshColliderContainer)
{
	// Bind mesh collider buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_MESH_COLLIDERS]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(P3MeshCollider) * meshColliderContainer.size(),
		meshColliderContainer.data(), GL_STATIC_READ);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDContainer[P3_AABBS]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(AabbGpuPackage), nullptr, GL_DYNAMIC_COPY);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0u, mSsboIDContainer[P3_MESH_COLLIDERS]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1u, mSsboIDContainer[P3_AABBS]);

	glUseProgram(mComputeProgramIDContainer[P3_UPDATE_AABBS]);

	glDispatchCompute(GLuint(1), GLuint(1), GLuint(1));

	// GLuint uniformIndex = glGetUniformBlockIndex(mComputeProgramIDContainer[P3_EVEN_ODD_SORT], "evenOrOdd");
	// glUniform1ui(uniformIndex, meshColliderContainer.size());

	CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0u, mSsboIDContainer[P3_UPDATE_AABBS]));

	// glUseProgram(mComputeProgramIDContainer[P3_DETECT_PAIRS]);

	// glDispatchCompute(1u, 1u, 1u);
	// glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
}

void P3OpenGLComputeBroadPhase::resetAtomicCounter()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	GLuint *pMappedBufferMemory = (GLuint *)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	memset(pMappedBufferMemory, 0, sizeof(GLuint));
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}