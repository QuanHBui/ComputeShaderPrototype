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

	GLuint get();

	void reset();
	void clear();

private:
	GLuint mAtomicBufferID = 0u;
};

#endif