#include "AtomicCounter.h"

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
	return *mpAtomicCounter;
}

void AtomicCounter::reset()
{
	*mpAtomicCounter = 0u;
}

void AtomicCounter::clear()
{
	glUnmapBuffer(mAtomicBufferID);
	glDeleteBuffers(1, &mAtomicBufferID);
}