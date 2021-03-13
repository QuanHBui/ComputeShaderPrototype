#include "P3OpenGLComputeNarrowPhase.h"

#include "P3NarrowPhaseCommon.h"
#include "OpenGLUtils.h"

void P3OpenGLComputeNarrowPhase::init(GLuint boxCollidersID, GLuint collisionPairsID)
{
	assert(boxCollidersID && collisionPairsID && "Invalid input buffer handles.");

	mSsboIDs[Buffer::BOX_COLLIDER]   = boxCollidersID;
	mSsboIDs[Buffer::COLLISION_PAIR] = collisionPairsID;

	initShaderPrograms();
	initGpuBuffers();

	mAtomicCounter.init();
}

void P3OpenGLComputeNarrowPhase::initGpuBuffers()
{
	// BoxCollider and CollisionPair buffers should already on the GPU from broadphase already.
	// Only need to allocate buffer for the manifolds
	GLuint temp[] = { 0, 0 };
	glGenBuffers(cNarrowPhaseSsboCount, temp);

	mSsboIDs[Buffer::MANIFOLD_FRONT] = temp[0];
	mSsboIDs[Buffer::MANIFOLD_BACK]  = temp[1];

	GLbitfield mapFlags = GL_MAP_READ_BIT
						| GL_MAP_PERSISTENT_BIT
						| GL_MAP_COHERENT_BIT;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDs[Buffer::MANIFOLD_FRONT]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(ManifoldGpuPackage), nullptr, mapFlags);

	// The pointer prob is pointing to pinned memory.
	mpManifoldPkg[0] = static_cast<ManifoldGpuPackage *>(glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(ManifoldGpuPackage),
		mapFlags
	));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDs[Buffer::MANIFOLD_BACK]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(ManifoldGpuPackage), nullptr, mapFlags);

	// The pointer prob is pointing to pinned memory.
	mpManifoldPkg[1] = static_cast<ManifoldGpuPackage *>(glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(ManifoldGpuPackage),
		mapFlags
	));
}

void P3OpenGLComputeNarrowPhase::step()
{
	GLuint currProgID = mComputeProgIDs[ComputeShader::SAT];
	glUseProgram(currProgID);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSsboIDs[Buffer::BOX_COLLIDER]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSsboIDs[Buffer::COLLISION_PAIR]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mSsboIDs[Buffer::MANIFOLD_FRONT]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mSsboIDs[Buffer::MANIFOLD_BACK]);

	mAtomicCounter.bindTo(0);

	glDispatchCompute(GLuint(1u), GLuint(1u), GLuint(1u));
	glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);

	glFinish();

	// The front buffer always has the final result, the back buffer should have the previous results
	mpManifoldPkg[mFrontBufferIdx]->misc.x = mAtomicCounter.get();

	mAtomicCounter.reset();
	glUseProgram(0u);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0u);
}
