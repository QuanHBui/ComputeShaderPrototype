#include "P3OpenGLComputeNarrowPhase.h"

#include <glad/glad.h>

void P3OpenGLComputeNarrowPhase::init(GLuint boxCollidersID, GLuint collisionPairsID)
{
	assert(boxCollidersID && collisionPairsID && "Invalid input buffer handles.");

	mBoxCollidersID = boxCollidersID;
	mCollisionPairsID = collisionPairsID;

	initShaderPrograms();
	initGpuBuffers();

	mAtomicCounter.init();
}

void P3OpenGLComputeNarrowPhase::initGpuBuffers()
{
	// BoxCollider and CollisionPair buffers should already on the GPU from broadphase already.
	// Only need to allocate 2 buffers: 1 for the manifolds, and 1 for the MTV's.
	glGenBuffers(2u, mSsboIDs);

	GLbitfield mapFlags = GL_MAP_READ_BIT
						| GL_MAP_PERSISTENT_BIT
						| GL_MAP_COHERENT_BIT;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDs[MANIFOLDS]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(ManifoldGpuPackage), nullptr, mapFlags);

	mpManifoldGpuPackage = static_cast<ManifoldGpuPackage *>(glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(ManifoldGpuPackage),
		mapFlags
	));
}

ManifoldGpuPackage const &P3OpenGLComputeNarrowPhase::step(uint16_t boxCollidersSize)
{
	mAtomicCounter.reset();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0u, mBoxCollidersID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1u, mCollisionPairsID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2u, mSsboIDs[MANIFOLDS]);

	mAtomicCounter.bindTo(3u);

	GLuint currProgID = mComputeProgramIDs[SAT];
	glUseProgram(currProgID);

	glDispatchCompute(GLuint(1u), GLuint(1u), GLuint(1u));
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glUseProgram(0u);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0u);

	return *mpManifoldGpuPackage;
}
