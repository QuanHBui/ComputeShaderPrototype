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

inline void quickCheckGLError()
{
	assert(glGetError() == GL_NO_ERROR);
}

void APIENTRY debugOutputMessageCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *, const void *);

inline void checkAndEnableDebugOutput()
{
	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(debugOutputMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
}
}
#endif // OPENGL_UTILS