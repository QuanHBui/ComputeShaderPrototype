#pragma once

#ifndef ATOMIC_COUNTER_H
#define ATOMIC_COUNTER_H

#include <glad/glad.h>

class AtomicCounter
{
public:
	void init();

	void bind()
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mAtomicBufferID);
	}

	void bindTo(GLuint bindIdx)
	{
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bindIdx, mAtomicBufferID);
	}

	void unbind()
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0u);
	}

	// Since this is created with coherent bit, no need to sync
	GLuint get()
	{
		return *mpAtomicCounter;
	}

	void reset()
	{
		*mpAtomicCounter = 0u;
	}

	void clear()
	{
		glUnmapBuffer(mAtomicBufferID);
		glDeleteBuffers(1, &mAtomicBufferID);
	}

private:
	GLuint mAtomicBufferID = 0u;
	GLuint *mpAtomicCounter = nullptr;
};

#endif
