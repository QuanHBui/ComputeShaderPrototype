#pragma once

#ifndef COMPUTE_PROGRAM_H
#define COMPUTE_PROGRAM_H

#include <glad/glad.h>
#include <string>

struct DispatchIndirectCommand
{
	GLuint numGroupsX;
	GLuint numGroupsY;
	GLuint numGroupsZ;
};

GLuint createComputeProgram(std::string const &);

#endif // COMPUTE_PROGRAM_H