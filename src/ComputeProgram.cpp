#include "ComputeProgram.h"

#include <stdexcept>

#include "GLSL.h"
#include "Program.h"

GLuint createComputeProgram(std::string const &shaderName)
{
	// Load the compute shader
	std::string shaderString = readFileAsString(shaderName);
	const char *shader = shaderString.c_str();
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &shader, nullptr);

	GLint success = 0;
	CHECKED_GL_CALL(glCompileShader(computeShader));

	// Check for compile status
	CHECKED_GL_CALL(glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success));
	if (!success)
	{
		GLSL::printShaderInfoLog(computeShader);
		CHECKED_GL_CALL(glDeleteShader(computeShader));
		throw std::runtime_error("Error compiling compute shader " + shaderName + ". Shader object will be deleted.");
	}

	GLuint programID = glCreateProgram();
	CHECKED_GL_CALL(glAttachShader(programID, computeShader));
	CHECKED_GL_CALL(glLinkProgram(programID));

	// Check for linking status
	CHECKED_GL_CALL(glGetProgramiv(programID, GL_LINK_STATUS, &success));
	if (!success)
	{
		GLSL::printShaderInfoLog(computeShader);
		CHECKED_GL_CALL(glDetachShader(programID, computeShader));
		CHECKED_GL_CALL(glDeleteShader(computeShader));
		throw std::runtime_error("Error compiling compute shader " + shaderName + ". Shader object will be deleted.");
	}

	// Detach and delete compute shader after linking
	CHECKED_GL_CALL(glDetachShader(programID, computeShader));
	CHECKED_GL_CALL(glDeleteShader(computeShader));

	return programID;
}