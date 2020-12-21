#pragma once

#ifndef OPENGL_UTILS
#define OPENGL_UTILS

#include <string>

std::string readFileAsString(const std::string &fileName);
void getComputeGroupInfo();
void getUboInfo();

#endif // OPENGL_UTILS