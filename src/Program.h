/**
 * Program object used to compile shader objects.
 *
 * @author: unknown. Assuming professors from Cal Poly Computer Science department.
 * @author: Slightly modified by Quan Bui.
 */

#pragma once

#ifndef PROGRAM_H
#define PROGRAM_H

#include <map>
#include <string>
#include <glad/glad.h>

std::string readFileAsString(const std::string &fileName);

class Program
{
public:
	~Program()
	{
		// Shader objects deletion should already be handled by deferred deletion
		if (pid)
			glDeleteProgram(pid);
	}

	GLuint getPID() const { return pid; }
	void setVerbose(const bool v) { verbose = v; }
	bool isVerbose() const { return verbose; }

	void setShaderNames(const std::string &, const std::string &);
	virtual void init();

	virtual void bind() const;
	virtual void unbind() const;

	void addAttribute(const std::string &);
	void addUniform(const std::string &);
	GLint getAttribute(const std::string &) const;
	GLint getUniform(const std::string &) const;

protected:
	std::string vShaderName;
	std::string fShaderName;

private:
	GLuint pid = 0u;
	std::map<std::string, GLint> attributes;
	std::map<std::string, GLint> uniforms;
	bool verbose = true;
};

#endif // PROGRAM_H