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
	glGenBuffers(3, temp);

	mSsboIDs[Buffer::MANIFOLD] = temp[0];
	mSsboIDs[Buffer::GLOBAL] = temp[1];
	mSsboIDs[Buffer::EXPERIMENTAL] = temp[2];

	GLbitfield mapFlags = GL_MAP_READ_BIT
						| GL_MAP_PERSISTENT_BIT
						| GL_MAP_COHERENT_BIT;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDs[Buffer::MANIFOLD]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(ManifoldGpuPackage), nullptr, mapFlags);

	// The pointer prob is pointing to pinned memory.
	mpManifoldGpuPackage = static_cast<ManifoldGpuPackage *>(glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER,
		0,
		sizeof(ManifoldGpuPackage),
		mapFlags
	));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDs[Buffer::GLOBAL]);
	glBufferStorage(
		GL_SHADER_STORAGE_BUFFER,
		sizeof(glm::vec4) * cMaxColliderCount * cBoxColliderFaceCount,
		nullptr,
		0
	);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboIDs[Buffer::EXPERIMENTAL]);
	//glBufferStorage(
	//	GL_SHADER_STORAGE_BUFFER,
	//	sizeof(ManifoldPackage),
	//	nullptr,
	//	0
	//);
}

ManifoldGpuPackage const &P3OpenGLComputeNarrowPhase::step(uint16_t boxCollidersSize)
{
	GLuint currProgID = mComputeProgIDs[ComputeShader::SAT];
	glUseProgram(currProgID);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSsboIDs[Buffer::BOX_COLLIDER]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSsboIDs[Buffer::COLLISION_PAIR]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mSsboIDs[Buffer::MANIFOLD]);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mSsboIDs[Buffer::EXPERIMENTAL]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mSsboIDs[Buffer::GLOBAL]);

	mAtomicCounter.bindTo(0);

	glDispatchCompute(GLuint(1u), GLuint(1u), GLuint(1u));
	GLsync syncObj = oglutils::lock();
	mAtomicCounter.lock();

	mpManifoldGpuPackage->misc.x = mAtomicCounter.get();

	mAtomicCounter.reset();
	glUseProgram(0u);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0u);

	// Wait till the manifold buffer done being used by the dispatch call, just bc it's coherent
	//  doesn't mean ALL of its data are the most up-to-date.
	oglutils::wait(syncObj);
	return *mpManifoldGpuPackage;
}
