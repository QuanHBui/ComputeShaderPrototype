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
		result.reserve((size_t) fileHandle.tellg());
		fileHandle.seekg(0, std::ios::beg);

		result.assign((std::istreambuf_iterator<char>(fileHandle)), std::istreambuf_iterator<char>());
	}
	else {
		std::cerr << "Could not open file: '" << fileName << "'\n";
	}

	return result;
}

Program::~Program()
{
	// Shader objects deletion should already be handled by deferred deletion
	if (pid) {
		glDeleteProgram(pid);
	}
}

void Program::setShaderNames(const std::string &v, const std::string &f)
{
	vShaderName = v;
	fShaderName = f;
}

bool Program::init()
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
			std::cerr << "Error compiling vertex shader " << vShaderName << '\n'
				<< "Shader object will be deleted\n";
		}

		glDeleteShader(vs);

		return false;
	}

	// Compile fragment shader
	CHECKED_GL_CALL(glCompileShader(fs));
	CHECKED_GL_CALL(glGetShaderiv(fs, GL_COMPILE_STATUS, &success));
	if (!success) {
		if (isVerbose()) {
			GLSL::printShaderInfoLog(fs);
			std::cerr << "Error compiling fragment shader " << fShaderName << '\n'
				<< "Shader object will be deleted\n";
		}

		glDeleteShader(fs);

		return false;
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
			std::cerr << "Error linking shaders " << vShaderName << " and " << fShaderName << '\n'
				<< "Program object and its associated shader objects will be deleted\n";
		}

		glDeleteProgram(pid);
		pid = 0u;

		glDetachShader(pid, vs);
		glDetachShader(pid, fs);

		glDeleteShader(vs);
		glDeleteShader(fs);

		return false;
	}

	// Detach shaders after successful link
	glDetachShader(pid, vs);
	glDetachShader(pid, fs);

	// Deferred deletion of shaders
	glDeleteShader(vs);
	glDeleteShader(fs);

	return true;
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