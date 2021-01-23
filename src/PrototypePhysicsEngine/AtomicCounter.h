#pragma once

#ifndef ATOMIC_COUNTER_H
#define ATOMIC_COUNTER_H

#include <glad/glad.h>

class AtomicCounter
{
public:
	void init();

	void bind();
	void bindTo(GLuint);
	void unbind();

	// Since this is created with coherent bit, no need to sync
	GLuint get();

	void lock();
	void wait();

	void reset();
	void clear();

private:
	GLuint mAtomicBufferID = 0u;
	GLuint *mpAtomicCounter = nullptr;
	GLsync mSync;
};

#endif
