#include "OpenGLUtils.h"

#include <cstdio>
#include <glad/glad.h>

void getComputeGroupInfo()
{
	GLint workGroupCount[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupCount[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupCount[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupCount[2]);
	printf("Max global (total) work group counts x:%i y:%i z:%i\n",
		workGroupCount[0], workGroupCount[1], workGroupCount[2]);

	GLint workGroupSize[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSize[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSize[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSize[2]);
	printf("Max local (in one shader) work group size x:%i y:%i z:%i \n",
		workGroupSize[0], workGroupSize[1], workGroupSize[2]);

	GLint workGroupInvocations;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workGroupInvocations);
	printf("Max local work group invocations: %i\n", workGroupInvocations);

	GLint maxSsbBindings;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxSsbBindings);
	printf("Max shader storage buffer bindings: %i\n", maxSsbBindings);
}

void getUboInfo()
{
	GLint returnInt;

	// Each shader stage has a limit on the number of seperate uniform buffer binding locations
	{
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &returnInt);
		printf("Max vertex uniform blocks or binding locations: %i\n", returnInt);

		glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &returnInt);
		printf("Max geometry uniform blocks or binding locations: %i\n", returnInt);

		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &returnInt);
		printf("Max fragment uniform blocks or binding locations: %i\n", returnInt);
	}

	// Limitation on the available storage per uniform buffer
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &returnInt);
	printf("Max uniform block size (in bytes): %i\n", returnInt);

	// When bind uniform buffer with glBindBufferRange, offset field must be multiple of GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &returnInt);
	printf("Uniform buffer offset alignment: %i\n", returnInt);
}