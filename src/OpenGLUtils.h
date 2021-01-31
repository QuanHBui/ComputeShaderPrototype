#pragma once

#ifndef OPENGL_UTILS
#define OPENGL_UTILS

#include <cassert>
#include <glad/glad.h>
#include <string>

namespace oglutils
{
std::string readFileAsString(const std::string &fileName);
void getComputeShaderInfo();
void getUboInfo();

inline GLsync lock()
{
	return glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

inline void wait(GLsync syncObj)
{
	assert(syncObj);

	GLenum waitReturnStatus = GL_UNSIGNALED;

	while (waitReturnStatus != GL_ALREADY_SIGNALED && waitReturnStatus != GL_CONDITION_SATISFIED)
	{
		waitReturnStatus = glClientWaitSync(syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
	}

	glDeleteSync(syncObj);
}

inline void checkGLError()
{
	assert(glGetError() == GL_NO_ERROR);
}
}
#endif // OPENGL_UTILS