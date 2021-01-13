#pragma once

#ifndef BUFFER_H
#define BUFFER_H

#include <glad/glad.h>

// This is sort of a struct of array, which can be a good thing.
class Buffer
{
public:
	Buffer() {}
	Buffer(size_t size);

	void *getData();

	~Buffer();
private:
	GLuint bufferID;
};

#endif // BUFFER_H
