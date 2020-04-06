#pragma once

#ifndef __Texture__
#define __Texture__

#include <glad/glad.h>
#include <string>

class Texture
{
private:
	std::string filename;
	int width;
	int height;
	GLuint tid;
	GLint unit;

public:
	Texture();
	virtual ~Texture();

	GLint getUnit() const { return unit; }
	GLint getID() const { return tid;}

	void setFilename(const std::string &f) { filename = f; }
	void setUnit(GLint u) { unit = u; }
	void setWrapModes(GLint, GLint); // Must be called after init()
	void setFilterModes(GLint, GLint);

	void init();
	void bind(GLint);
	void unbind();
};

#endif // __Texture__
