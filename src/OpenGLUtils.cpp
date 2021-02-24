#include "OpenGLUtils.h"

#include <cstdio>
#include <fstream>
#include <iostream>

namespace oglutils
{
std::string readFileAsString(const std::string &fileName)
{
	std::string result;
	std::ifstream fileHandle(fileName);

	if (fileHandle.is_open())
	{
		fileHandle.seekg(0, std::ios::end);
		result.reserve((size_t)fileHandle.tellg());
		fileHandle.seekg(0, std::ios::beg);

		result.assign((std::istreambuf_iterator<char>(fileHandle)), std::istreambuf_iterator<char>());
	}
	else
	{
		throw std::runtime_error("Could not open file: " + fileName);
	}

	return result;
}

void getComputeShaderInfo()
{
	GLint intArray[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &intArray[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &intArray[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &intArray[2]);
	printf("Max global (total) work group counts x:%i y:%i z:%i\n",
		intArray[0], intArray[1], intArray[2]);

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &intArray[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &intArray[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &intArray[2]);
	printf("Max local (in one shader) work group size x:%i y:%i z:%i \n",
		intArray[0], intArray[1], intArray[2]);

	GLint justSomeInt;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &justSomeInt);
	printf("Max local work group invocations: %i\n", justSomeInt);

	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &justSomeInt);
	printf("Max shader storage buffer bindings: %i\n", justSomeInt);

	glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &justSomeInt);
	printf("Max total shared variables storage size (in bytes): %i\n", justSomeInt);
}

void getUboInfo()
{
	GLint returnInt;

	// Each shader stage has a limit on the number of seperate uniform buffer binding locations
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &returnInt);
	printf("Max vertex uniform blocks or binding locations: %i\n", returnInt);

	glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &returnInt);
	printf("Max geometry uniform blocks or binding locations: %i\n", returnInt);

	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &returnInt);
	printf("Max fragment uniform blocks or binding locations: %i\n", returnInt);

	// Limitation on the available storage per uniform buffer
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &returnInt);
	printf("Max uniform block size (in bytes): %i\n", returnInt);

	// When bind uniform buffer with glBindBufferRange, offset field must be multiple of GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &returnInt);
	printf("Uniform buffer offset alignment: %i\n", returnInt);
}

// @source: https://learnopengl.com/In-Practice/Debugging
void APIENTRY debugOutputMessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const char *message,
	const void *pUserParam )
{
	// Ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------\n"
		<< "Debug message (" << id << "): " << message << '\n';

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << '\n';

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << '\n';

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << '\n' << std::endl;

	__debugbreak();
}
}
