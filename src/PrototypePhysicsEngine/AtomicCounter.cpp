#include "AtomicCounter.h"

void AtomicCounter::init()
{
	glGenBuffers(1, &mAtomicBufferID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
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
	bind();

	GLuint *pCounter = (GLuint *)glMapBufferRange(
		GL_ATOMIC_COUNTER_BUFFER,
		0,
		sizeof(GLuint),
		GL_MAP_READ_BIT
	);
	GLuint counter = *pCounter;
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

	unbind();

	return counter;
}

void AtomicCounter::reset()
{
	bind();

	GLuint *pMappedBufferMemory = (GLuint *)glMapBufferRange(
		GL_ATOMIC_COUNTER_BUFFER,
		0,
		sizeof(GLuint),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT
	);
	memset(pMappedBufferMemory, 0, sizeof(GLuint));
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

	unbind();
}

void AtomicCounter::clear()
{
	glDeleteBuffers(1, &mAtomicBufferID);
}