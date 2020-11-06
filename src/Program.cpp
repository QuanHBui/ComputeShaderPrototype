/**
 * Implementation of Program object used to compile shader objects.
 *
 * @author: unknown. Assuming professors from Cal Poly Computer Science department.
 * @author: Slightly modified by Quan Bui.
 */

#include "Program.h"

#include <iostream>
#include <cassert>
#include <fstream>

#include "GLSL.h"

std::string readFileAsString(const std::string &fileName)
{
	std::string result;
	std::ifstream fileHandle(fileName);

	if (fileHandle.is_open()) {
		fileHandle.seekg(0, std::ios::end);
		result.reserve((size_t)fileHandle.tellg());
		fileHandle.seekg(0, std::ios::beg);

		result.assign((std::istreambuf_iterator<char>(fileHandle)), std::istreambuf_iterator<char>());
	}
	else {
		throw std::runtime_error("Could not open file: " + fileName);
	}

	return result;
}

void Program::setShaderNames(const std::string &v, const std::string &f)
{
	vShaderName = v;
	fShaderName = f;
}

void Program::init()
{
	GLint success;

	// Create shader handles
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	// Read shader sources
	std::string vShaderString = readFileAsString(vShaderName);
	std::string fShaderString = readFileAsString(fShaderName);
	const char *vshader = vShaderString.c_str();
	const char *fshader = fShaderString.c_str();
	CHECKED_GL_CALL(glShaderSource(vs, 1, &vshader, NULL));
	CHECKED_GL_CALL(glShaderSource(fs, 1, &fshader, NULL));

	// Compile vertex shader
	CHECKED_GL_CALL(glCompileShader(vs));
	CHECKED_GL_CALL(glGetShaderiv(vs, GL_COMPILE_STATUS, &success));
	if (!success) {
		if (isVerbose()) {
			GLSL::printShaderInfoLog(vs);
		}

		glDeleteShader(vs);
		throw std::runtime_error("Error compiling vertex shader " + vShaderName);
	}

	// Compile fragment shader
	CHECKED_GL_CALL(glCompileShader(fs));
	CHECKED_GL_CALL(glGetShaderiv(fs, GL_COMPILE_STATUS, &success));
	if (!success) {
		if (isVerbose()) {
			GLSL::printShaderInfoLog(fs);
		}

		glDeleteShader(fs);
		throw std::runtime_error("Error compiling fragment shader " + fShaderName);
	}

	// Create the program and link
	pid = glCreateProgram();
	CHECKED_GL_CALL(glAttachShader(pid, vs));
	CHECKED_GL_CALL(glAttachShader(pid, fs));
	CHECKED_GL_CALL(glLinkProgram(pid));
	CHECKED_GL_CALL(glGetProgramiv(pid, GL_LINK_STATUS, &success));
	if (!success) {
		if (isVerbose()) {
			GLSL::printProgramInfoLog(pid);
		}

		glDeleteProgram(pid);
		pid = 0u;

		glDetachShader(pid, vs);
		glDetachShader(pid, fs);

		glDeleteShader(vs);
		glDeleteShader(fs);

		throw std::runtime_error("Error linking shaders " + vShaderName + " and " + fShaderName);
	}

	// Detach shaders after successful link
	glDetachShader(pid, vs);
	glDetachShader(pid, fs);

	// Deferred deletion of shaders
	glDeleteShader(vs);
	glDeleteShader(fs);
}

void Program::bind()
{
	CHECKED_GL_CALL(glUseProgram(pid));
}

void Program::unbind()
{
	CHECKED_GL_CALL(glUseProgram(0));
}

void Program::addAttribute(const std::string &name)
{
	attributes[name] = GLSL::getAttribLocation(pid, name.c_str(), isVerbose());
}

void Program::addUniform(const std::string &name)
{
	uniforms[name] = GLSL::getUniformLocation(pid, name.c_str(), isVerbose());
}

GLint Program::getAttribute(const std::string &name) const
{
	std::map<std::string, GLint>::const_iterator attribute = attributes.find(name.c_str());
	if (attribute == attributes.end()) {
		if (isVerbose()) {
			std::cout << name << " is not an attribute variable" << std::endl;
		}

		return -1;
	}

	return attribute->second;
}

GLint Program::getUniform(const std::string &name) const
{
	std::map<std::string, GLint>::const_iterator uniform = uniforms.find(name.c_str());
	if (uniform == uniforms.end()) {
		if (isVerbose()) {
			std::cout << name << " is not a uniform variable" << std::endl;
		}

		return -1;
	}

	return uniform->second;
}