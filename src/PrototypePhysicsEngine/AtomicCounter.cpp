#include "AtomicCounter.h"

#include <cassert>

void AtomicCounter::init()
{
	glGenBuffers(1, &mAtomicBufferID);

	GLbitfield mapFlags = GL_MAP_READ_BIT
						| GL_MAP_WRITE_BIT
						| GL_MAP_PERSISTENT_BIT
						| GL_MAP_COHERENT_BIT;

	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	glBufferStorage(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, mapFlags);

	mpAtomicCounter = static_cast<GLuint *>(glMapBufferRange(
		GL_ATOMIC_COUNTER_BUFFER,
		0,
		sizeof(GLuint),
		mapFlags
	));

	unbind();
}
