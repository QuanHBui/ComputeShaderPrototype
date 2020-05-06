#include "Application.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "stb_image.h"

void getComputeGroupInfo()
{
	GLint work_grp_cnt[3];

	CHECKED_GL_CALL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]));
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("max global (total) work group counts x:%i y:%i z:%i\n",
		work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	GLint work_grp_size[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	printf("max local (in one shader) work group size x:%i y:%i z:%i \n",
		work_grp_size[0], work_grp_size[1], work_grp_size[2]);

	GLint work_grp_inv;

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	printf("max local work group invocations %i\n", work_grp_inv);

	GLint max_shader_storage_buffer_bindings;

	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &max_shader_storage_buffer_bindings);
	printf("max shader storage buffer bindings %i\n", max_shader_storage_buffer_bindings);
}

Application::~Application()
{
	std::cout << "\nApplication is safely destroyed.\n";
	windowManager = nullptr;
	CHECKED_GL_CALL(glDeleteBuffers(1, &ssboGPU_id));
	CHECKED_GL_CALL(glDeleteProgram(computeProgram_id));
}

void Application::initGeom()
{
	std::vector<tinyobj::shape_t> bunnyShapes;
	std::vector<tinyobj::material_t> bunnyMaterials;
	std::string errStrBunny;

	bool bunnyLoadCheck = tinyobj::LoadObj(	bunnyShapes, bunnyMaterials,
											errStrBunny, "../resources/models/bunny.obj");
	if (!bunnyLoadCheck)
		std::cerr << errStrBunny << std::endl;
	else {
		meshContainer.emplace_back(std::make_unique<Shape>());
		meshContainer.back()->createShape(bunnyShapes.at(0));
		meshContainer.back()->init();
	}

	// Check for the size of the bunny mesh vertex buffer
	printf("\nSize of bunny vertex buffer: %d\nSize of bunny element buffer: %d\n",
			meshContainer.back()->posBuf.size(), meshContainer.back()->eleBuf.size());
	fflush(stdout);
}

// Initialize SSBO with the vertex and element buffers from loading mesh obj, pre-transformed
void Application::initSSBO()
{
	// We prep ssbo with only one vertex and elemeent buffers because we "clone" one mesh
	std::vector<float> vertexBuffer_A = meshContainer.back()->posBuf;
	std::vector<unsigned int> elementBuffer = meshContainer.back()->eleBuf;

	// We are going to test the first 1024 triangles of the bunny mesh
	for (int i = 0; i < NUM_TRIANGLES; i++) {
		ssboCPUMEM.transformedVertexBuffer_A[i] = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
		ssboCPUMEM.transformedVertexBuffer_B[i] = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
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

void Application::initRenderProgram()
{
	renderProgramPtr = std::make_unique<Program>();
	renderProgramPtr->setVerbose(true);
	renderProgramPtr->setShaderNames("../resources/shaders/vs.glsl", "../resources/shaders/fs.glsl");
	renderProgramPtr->init();
}

// General OGL initialization - set OGL state here
void Application::initComputeProgram()
{
	GLSL::checkVersion();

	getComputeGroupInfo();

	// Load the compute shader
	std::string shaderString = readFileAsString("../resources/shaders/cs.glsl");
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

void Application::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

// Bind SSBO to compute program and dispatch work group
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

// Bind SSBO to render program and draw
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

	float aspect = width/(float)height;

	glm::mat4 projection = glm::mat4(1.0f) * glm::perspective(45.0f, aspect, 0.01f, 1000.0f);
	glm::mat4 model = glm::mat4(1.0f);

	// renderProgramPtr->bind();
	CHECKED_GL_CALL(glGetProgramResourceIndex(renderProgramPtr->getPID(), GL_UNIFORM_BLOCK, "TransformMatrixBlock"));

	GLuint pid = renderProgramPtr->getPID();

	std::cout << "Location for interface block: "
		<< "projection "<< glGetProgramResourceIndex(pid, GL_UNIFORM, "TransformMatrixBlock.projection") << std::endl
		<< "view " << glGetProgramResourceIndex(pid, GL_UNIFORM, "TransformMatrixBlock.view") << std::endl
		<< "model " << glGetProgramResourceIndex(pid, GL_UNIFORM, "TransformMatrixBlock.model") << std::endl
		<< glGetAttribLocation(pid, "vertPos") << std::endl
		<< glGetAttribLocation(pid, "vertNor") << std::endl
		<< std::endl;
	std::cout << GL_INVALID_INDEX << std::endl;
}