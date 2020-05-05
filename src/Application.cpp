#include "Application.h"

#include <iostream>

#include "Program.h"
#include "GLSL.h"
#include "stb_image.h"

Application::~Application()
{
	std::cout << "\nApplication is safely destroyed.\n";
	windowManager = nullptr;
	CHECKED_GL_CALL(glDeleteBuffers(1, &ssboGPU_id));
	CHECKED_GL_CALL(glDeleteProgram(computeProgram_id));
}

// General OGL initialization - set OGL state here
void Application::init()
{
	GLSL::checkVersion();

	//load the compute shader
	std::string shaderString = readFileAsString("../resources/shaders/compute.glsl");
	const char *shader = shaderString.c_str();
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &shader, nullptr);

	GLint success = 0;
	CHECKED_GL_CALL(glCompileShader(computeShader));

	// Check for compile status
	CHECKED_GL_CALL(glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success));
	if (!success) {
		GLSL::printShaderInfoLog(computeShader);
		std::cerr << "Error compiling compute shader. Shader object will be deleted.\n";
		CHECKED_GL_CALL(glDeleteShader(computeShader));
		exit(EXIT_FAILURE);
	}

	computeProgram_id = glCreateProgram();
	CHECKED_GL_CALL(glAttachShader(computeProgram_id, computeShader));
	CHECKED_GL_CALL(glLinkProgram(computeProgram_id));

	// Check for linking status
	CHECKED_GL_CALL(glGetProgramiv(computeProgram_id, GL_LINK_STATUS, &success));
	if (!success) {
		GLSL::printShaderInfoLog(computeShader);
		std::cerr << "Error linking compute shader. Compute program and shader will be deleted.\n";
		CHECKED_GL_CALL(glDetachShader(computeProgram_id, computeShader));
		CHECKED_GL_CALL(glDeleteShader(computeShader));
		exit(EXIT_FAILURE);
	}

	// Detach and delete compute shader after linking
	CHECKED_GL_CALL(glDetachShader(computeProgram_id, computeShader));
	CHECKED_GL_CALL(glDeleteShader(computeShader));
}

void Application::initGeom()
{

}

void Application::initSSBO()
{
	for (int i = 0; i < STAR_COUNT; i++) {
		ssboCPUMEM.vertexBuffer_A[i] = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
		ssboCPUMEM.vertexBuffer_B[i] = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
		ssboCPUMEM.elementBuffer_A[i] = glm::uvec4{ 0u, 0u, 0u, 0u };
		ssboCPUMEM.elementBuffer_B[i] = glm::uvec4{ 0u, 0u, 0u, 0u };
	}

	// Make an SSBO
	glGenBuffers(1, &ssboGPU_id);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGPU_id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssboCPUMEM), &ssboCPUMEM, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboGPU_id);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind
}

void Application::compute()
{
	GLuint block_index = 0;
	block_index = glGetProgramResourceIndex(computeProgram_id, GL_SHADER_STORAGE_BLOCK, "shader_data");

	GLuint ssbo_binding_point_index = 0;
	CHECKED_GL_CALL(glShaderStorageBlockBinding(computeProgram_id, block_index, ssbo_binding_point_index));
	CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboGPU_id));

	CHECKED_GL_CALL(glUseProgram(computeProgram_id));
	CHECKED_GL_CALL(glDispatchCompute(2, 1, 1));
	CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));

	// Copy data back to CPU MEM
	CHECKED_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGPU_id));
	GLvoid *p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	int siz = sizeof(SSBO);
	memcpy(&ssboCPUMEM, p, siz);
	CHECKED_GL_CALL(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));

	// Unbind storage buffer and program objects
	CHECKED_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
	CHECKED_GL_CALL(glUseProgram(0));
}

void Application::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void Application::render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

	// Prevent assertion when minimize the window
	if (!width && !height) return;

	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}