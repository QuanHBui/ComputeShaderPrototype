#pragma once

#ifndef COMPUTE_PROGRAM_H
#define COMPUTE_PROGRAM_H

#include <glad/glad.h>

void getComputeGroupInfo();
GLuint createComputeProgram(const char *);

#endif // COMPUTE_PROGRAM_H