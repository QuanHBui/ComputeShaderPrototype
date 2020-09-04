#include "ComputeProgram.h"

#include <stdexcept>

#include "GLSL.h"
#include "Program.h"

void getComputeGroupInfo()
{
	GLint work_grp_cnt[3];

	CHECKED_GL_CALL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]));
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("Max global (total) work group counts x:%i y:%i z:%i\n",
		work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	GLint work_grp_size[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	printf("Max local (in one shader) work group size x:%i y:%i z:%i \n",
		work_grp_size[0], work_grp_size[1], work_grp_size[2]);

	GLint work_grp_inv;

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	printf("Max local work group invocations %i\n", work_grp_inv);

	GLint max_shader_storage_buffer_bindings;

	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &max_shader_storage_buffer_bindings);
	printf("Max shader storage buffer bindings %i\n", max_shader_storage_buffer_bindings);
}

GLuint createComputeProgram(const char *shaderName)
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
		throw std::runtime_error("Error compiling compute shader. Shader object will be deleted.\n");
		CHECKED_GL_CALL(glDeleteShader(computeShader));
		exit(EXIT_FAILURE);
	}

	GLuint programID = glCreateProgram();
	CHECKED_GL_CALL(glAttachShader(programID, computeShader));
	CHECKED_GL_CALL(glLinkProgram(programID));

	// Check for linking status
	CHECKED_GL_CALL(glGetProgramiv(programID, GL_LINK_STATUS, &success));
	if (!success)
	{
		GLSL::printShaderInfoLog(computeShader);
		throw std::runtime_error("Error linking compute shader. Compute program and shader will be deleted.\n");
		CHECKED_GL_CALL(glDetachShader(programID, computeShader));
		CHECKED_GL_CALL(glDeleteShader(computeShader));
		exit(EXIT_FAILURE);
	}

	// Detach and delete compute shader after linking
	CHECKED_GL_CALL(glDetachShader(programID, computeShader));
	CHECKED_GL_CALL(glDeleteShader(computeShader));

	return programID;
}