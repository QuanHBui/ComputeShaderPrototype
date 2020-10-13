#pragma once

#ifndef COMPUTE_PROGRAM_H
#define COMPUTE_PROGRAM_H

#include <glad/glad.h>
#include <string>

GLuint createComputeProgram(std::string const &);

#endif // COMPUTE_PROGRAM_H