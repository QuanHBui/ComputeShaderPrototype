#include "AtomicCounter.h"

#include <cassert>

void AtomicCounter::init()
{
	glGenBuffers(1, &mAtomicBufferID);

	GLbitfield mapFlags = GL_MAP_READ_BIT
						| GL_MAP_WRITE_BIT
						| GL_MAP_PERSISTENT_BIT
						| GL_MAP_COHERENT_BIT;

	bind();
	glBufferStorage(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, mapFlags);

	mpAtomicCounter = static_cast<GLuint *>(glMapBufferRange(
		GL_ATOMIC_COUNTER_BUFFER,
		0,
		sizeof(GLuint),
		mapFlags
	));

	unbind();
}

void AtomicCounter::bind()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
}

void AtomicCounter::bindTo(GLuint bindIdx)
{
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bindIdx, mAtomicBufferID);
}

void AtomicCounter::unbind()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0u);
}

GLuint AtomicCounter::get()
{
	wait();
	return *mpAtomicCounter;
}

// Should be used to insert a fence into the command stream. Be careful where to put it.
void AtomicCounter::lock()
{
	// Delete any existing sync object. Do we really need to do this?
	if (mSync)
		glDeleteSync(mSync);

	mSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

// Force sync function, the client/host would wait until the server/device done using
//  the buffer to update/modify.
void AtomicCounter::wait()
{
	assert(mSync);

	GLenum waitReturnStatus = GL_UNSIGNALED;

	while (waitReturnStatus != GL_ALREADY_SIGNALED && waitReturnStatus != GL_CONDITION_SATISFIED)
	{
		waitReturnStatus = glClientWaitSync(mSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
	}
}

void AtomicCounter::reset()
{
	wait();
	*mpAtomicCounter = 0u;
}

void AtomicCounter::clear()
{
	glUnmapBuffer(mAtomicBufferID);
	glDeleteBuffers(1, &mAtomicBufferID);
}