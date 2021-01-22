#pragma once

#ifndef BUFFER_H
#define BUFFER_H

#include <glad/glad.h>

/**
 * This is sort of a struct of array, which can be a good thing. This class is
 *  simply for OpenGL bookeeping. It uses the concept of persistent mapping for
 *  efficient data streaming. Later on, it can be further improve with triple
 *  buffering.
 *
 * For extreme optimization, a huge buffer will be allocated and then it will be
 *  chopped into smaller sections to use. Remember, allocation is expensive.
 *
 * Can go 2 directions: 
 *  (1) The C route: with void pointers and all that jazz.
 *  (2) The C++ route: with template and more overheads.
 *
 * This class is a template. Compilation might be slow. Using this class also
 *  reduce the amount of customizations comparing to straight up calling OpenGL
 *  function. But, we'll have to see if it is worth using this class.
 *
 * EXTREMELY EXPERIMENTAL!
 */
template<typename T>
class Buffer
{
public:
	Buffer()
	{

	}

	void *getData();

	~Buffer();
private:
	GLuint bufferID;
};

#endif // BUFFER_H
