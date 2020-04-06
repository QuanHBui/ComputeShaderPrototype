/**
 * Program object used to compile shader objects. This program object can have mix-and-match
 *  code for different shader stages dynamically post-linking. This means no need for multiple
 *  programs in application for different shaders. Each program object only holds compiled code
 *  for 1 single shader stage. Finally, only 1 program pipeline will be used for the render
 *  call in application.
 *  a.k.a the concept of separate program:
 *  https://www.khronos.org/opengl/wiki/Shader_Compilation#Separate_programs
 *
 * @author: unknown. Assuming professors from Cal Poly Computer Science department.
 *  Modified by Quan Bui.
 * @version: 04/05/2020
 */

#pragma once

#ifndef LAB471_PROGRAM_H_INCLUDED
#define LAB471_PROGRAM_H_INCLUDED

#include <map>
#include <string>
#include <glad/glad.h>

std::string readFileAsString(const std::string &fileName);

class Program
{
private:
	GLuint pid = static_cast<GLuint>(0);
	std::map<std::string, GLint> attributes;
	std::map<std::string, GLint> uniforms;
	bool verbose = true;

	bool compileShaders();
	bool attachShaders();
	bool linkShaders();

protected:
	std::string vShaderName;
	std::string fShaderName;

public:
	~Program();

	void setVerbose(const bool v) { verbose = v; }
	bool isVerbose() const { return verbose; }

	void setShaderNames(const std::string &, const std::string &);
	virtual bool init();
	virtual void bind();
	virtual void unbind();

	void addAttribute(const std::string &);
	void addUniform(const std::string &);
	GLint getAttribute(const std::string &) const;
	GLint getUniform(const std::string &) const;

	bool addGeometryShader();
	bool addComputeShader();

};

#endif // LAB471_PROGRAM_H_INCLUDED