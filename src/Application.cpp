#include "Application.h"

#include <iostream>

#include "Program.h"
#include "GLSL.h"
#include "stb_image.h"

// General OGL initialization - set OGL state here
void Application::init()
{
	GLSL::checkVersion();

	//load the compute shader
	std::string ShaderString = readFileAsString("../resources/compute.glsl");
	const char *shader = ShaderString.c_str();
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &shader, nullptr);

	GLint rc;
	CHECKED_GL_CALL(glCompileShader(computeShader));
	CHECKED_GL_CALL(glGetShaderiv(computeShader, GL_COMPILE_STATUS, &rc));
	if (!rc)	// error compiling the shader file
	{
		GLSL::printShaderInfoLog(computeShader);
		std::cout << "Error compiling fragment shader " << std::endl;
		exit(1);
	}

	computeProgram = glCreateProgram();
	glAttachShader(computeProgram, computeShader);
	glLinkProgram(computeProgram);
	glUseProgram(computeProgram);

	GLuint block_index = 0;
	block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "shader_data");
	GLuint ssbo_binding_point_index = 2;
	glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding_point_index);
}

void Application::initGeom()
{

}

void Application::initTex()
{
	std::string resourceDirectory = "../resources";
	int width, height, channels;
	char filepath[1000];
	//texture 1
	std::string str = resourceDirectory + "/Blue_Giant.jpg";
	strcpy_s(filepath, str.c_str());
	unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Make an SSBO
	for (int ii = 0; ii < STAR_COUNT; ii++) {
		ssbo_CPUMEM.dataA[ii] = glm::vec4(ii, 0.0, 0.0, 0.0);
		ssbo_CPUMEM.dataB[ii] = glm::vec4(0.0, 0.0, 0.0, 0.0);
	}
	glGenBuffers(1, &ssbo_GPU_id);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_GPU_id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssbo_data), &ssbo_CPUMEM, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_GPU_id);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
}

/* Note that any gl calls must always happen after a GL state is initialized */
void Application::initAtomic()
{
	glGenBuffers(1, &atomicsBuffer);
	// bind the buffer and define its initial storage capacity
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * 1, NULL, GL_DYNAMIC_DRAW);
	// unbind the buffer
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void Application::resetAtomic()
{
	GLuint *userCounters;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
	// map the buffer, userCounters will point to the buffers data
	userCounters = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER,
		0,
		sizeof(GLuint),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT
	);
	// set the memory to zeros, resetting the values in the buffer
	memset(userCounters, 0, sizeof(GLuint));
	// unmap the buffer
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}

void Application::readAtomic()
{
	GLuint *userCounters;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
	// again we map the buffer to userCounters, but this time for read-only access
	userCounters = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER,
		0,
		sizeof(GLuint),
		GL_MAP_READ_BIT
	);
	// copy the values to other variables because...
	std::cout << std::endl << *userCounters << std::endl;
	// ... as soon as we unmap the buffer
	// the pointer userCounters becomes invalid.
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}

void Application::compute()
{
	//print data before compute shader
	std::cout << "\n\nBUFFER BEFORE COMPUTE SHADER\n\n";
	//for (int i = 0; i < STARCOUNT; i++)
		//cout << "dataA: " << ssbo_CPUMEM.dataA[i].x << ", " << ssbo_CPUMEM.dataA[i].y << ", " << ssbo_CPUMEM.dataA[i].z << ", " << ssbo_CPUMEM.dataA[i].w << "   dataB: "<<ssbo_CPUMEM.dataB[i].x << ", " << ssbo_CPUMEM.dataB[i].y << ", " << ssbo_CPUMEM.dataB[i].z << ssbo_CPUMEM.dataB[i].w << endl;

	GLuint block_index = 0;
	block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "shader_data");
	GLuint ssbo_binding_point_index = 0;
	glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding_point_index);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_GPU_id);
	glUseProgram(computeProgram);
	//activate atomic counter
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicsBuffer);

	glDispatchCompute((GLuint)2, (GLuint)1, 1);				//start compute shader
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

	//copy data back to CPU MEM

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_GPU_id);
	GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	int siz = sizeof(ssbo_data);
	memcpy(&ssbo_CPUMEM,p, siz);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//print data after compute shader
	std::cout << "\n\nBUFFER AFTER COMPUTE SHADER\n\n";
	//for (int i = 0; i < STARCOUNT; i++)
	std::cout << "dataB: " << ssbo_CPUMEM.dataB[0].y << std::endl;
}