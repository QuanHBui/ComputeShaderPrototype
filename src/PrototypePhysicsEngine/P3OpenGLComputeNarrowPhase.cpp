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
	GLuint temp[] = { 0, 0, 0 };
	glGenBuffers(2, temp);

	mSsboIDs[Buffer::MANIFOLD]     = temp[0];
	mSsboIDs[Buffer::EXPERIMENTAL] = temp[1];

	GLbitfield mapFlags = GL_MAP_READ_BIT
						| GL_MAP_PERSISTENT_BIT
						| GL_MAP_COHERENT_BIT;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDs[Buffer::MANIFOLD]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(ManifoldGpuPackage), nullptr, mapFlags);

	// The pointer prob is pointing to pinned memory.
	mpManifoldPkg = static_cast<ManifoldGpuPackage *>(glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(ManifoldGpuPackage),
		mapFlags
	));

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDs[Buffer::EXPERIMENTAL]);
	//glBufferStorage(
	//	GL_SHADER_STORAGE_BUFFER,
	//	sizeof(ManifoldPackage),
	//	nullptr,
	//	0
	//);
}

void P3OpenGLComputeNarrowPhase::step()
{
	GLuint currProgID = mComputeProgIDs[ComputeShader::SAT];
	glUseProgram(currProgID);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSsboIDs[Buffer::BOX_COLLIDER]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSsboIDs[Buffer::COLLISION_PAIR]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mSsboIDs[Buffer::MANIFOLD]);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mSsboIDs[Buffer::EXPERIMENTAL]);

	mAtomicCounter.bindTo(0);

	glDispatchCompute(GLuint(1u), GLuint(1u), GLuint(1u));
	glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);

	mpManifoldPkg->misc.x = mAtomicCounter.get();

	mAtomicCounter.reset();
	glUseProgram(0u);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0u);
}
