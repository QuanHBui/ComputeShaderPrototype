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
	printf("max shader storage buffer bindings %i\n\n", max_shader_storage_buffer_bindings);
}

Application::~Application()
{
	std::cout << "\nApplication is safely destroyed.\n";
	windowManager = nullptr;
	CHECKED_GL_CALL(glDeleteBuffers(1, &ssboGPU_id));
	CHECKED_GL_CALL(glDeleteProgram(computeProgram_id));
}

void Application::printSSBO()
{
	const glm::vec4 *vertexBuffer_A = &ssboCPUMEM.vertexBuffer_A[0];
	const glm::vec4 *vertexBuffer_B = &ssboCPUMEM.vertexBuffer_B[0];
	const glm::uvec4 *elementBuffer_A = &ssboCPUMEM.elementBuffer_A[0];
	const glm::uvec4 *elementBuffer_B = &ssboCPUMEM.elementBuffer_B[0];
	const glm::mat4 &model_A = ssboCPUMEM.model_A;
	const glm::mat4 &model_B = ssboCPUMEM.model_B;

	printf("model_A:\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
			model_A[0][0], model_A[0][1], model_A[0][2], model_A[0][3],
			model_A[1][0], model_A[1][1], model_A[1][2], model_A[1][3],
			model_A[2][0], model_A[2][1], model_A[2][2], model_A[2][3],
			model_A[3][0], model_A[3][1], model_A[3][2], model_A[3][3]);

	printf("model_B:\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
			model_B[0][0], model_B[0][1], model_B[0][2], model_B[0][3],
			model_B[1][0], model_B[1][1], model_B[1][2], model_B[1][3],
			model_B[2][0], model_B[2][1], model_B[2][2], model_B[2][3],
			model_B[3][0], model_B[3][1], model_B[3][2], model_B[3][3]);

	// Print out nicely the first 5 elements of the ssbo
	for (int i = 0; i < 5; ++i) {
		std::cout << "vertexBuffer_A: "	<< vertexBuffer_A[i].x << ", "
										<< vertexBuffer_A[i].y << ", "
										<< vertexBuffer_A[i].z << ", "
										<< vertexBuffer_A[i].w << '\n';
		std::cout << "vertexBuffer_B: "	<< vertexBuffer_B[i].x << ", "
										<< vertexBuffer_B[i].y << ", "
										<< vertexBuffer_B[i].z << ", "
										<< vertexBuffer_B[i].w << '\n';
		std::cout << "elementBuffer_A: "	<< elementBuffer_A[i].x << ", "
											<< elementBuffer_A[i].y << ", "
											<< elementBuffer_A[i].z << ", "
											<< elementBuffer_A[i].w << "\n";
		std::cout << "elementBuffer_B: "	<< elementBuffer_B[i].x << ", "
											<< elementBuffer_B[i].y << ", "
											<< elementBuffer_B[i].z << ", "
											<< elementBuffer_B[i].w << "\n\n";
	}
	std::cout << std::endl;
}

void Application::interpretComputedSSBO()
{
	const glm::uvec4 *elementBuffer_A = &ssboCPUMEM.elementBuffer_A[0];
	const glm::uvec4 *elementBuffer_B = &ssboCPUMEM.elementBuffer_B[0];

	// 5522 is the number of triangles in the bunny mesh
	// Loop through the element buffer and if w component is 1, that
	//  triangle has collided

	// Two loops but this is for better cache locality. Computers love
	//  tightly packed arrays
	std::cout << "In mesh A, collided triangles are:\n";
	for (int i = 0; i < 5522; ++i) {
		if (elementBuffer_A[i].w == 1u) {
			std::cout << i;

			if (i != 5521) std::cout << ", ";
		}
	}
	std::cout << "\n\n";

	std::cout << "In mesh B, collided triangles are:\n";
	for (int j = 0; j < 5522; ++j) {
		if (elementBuffer_B[j].w == 1u) {
			std::cout << j;

			if (j != 5521) std::cout << ", ";
		}
	}
	std::cout << '\n' << std::endl;
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
	printf("\nSize of bunny vertex buffer: %zd\nSize of bunny element buffer: %zd\n\n",
			meshContainer.back()->posBuf.size(), meshContainer.back()->eleBuf.size());
	fflush(stdout);
}

// Initialize SSBO with the vertex and element buffers from loading mesh obj, pre-transformed
void Application::initSSBO()
{
	// We prep ssbo with only one vertex and element buffers because we "clone" one mesh
	const std::vector<float> &vertexBuffer = meshContainer.back()->posBuf;
	const std::vector<unsigned int> &elementBuffer = meshContainer.back()->eleBuf;

	// We are going to test the first 1024 triangles of the bunny mesh
	for (int i = 0; i < 2763; ++i) {
		// Last index is just for "padding." It might serve a purpose in the future
		ssboCPUMEM.vertexBuffer_A[i] = glm::vec4{	vertexBuffer[3 * i],
													vertexBuffer[(3 * i) + 1],
													vertexBuffer[(3 * i) + 2],
													0.0f };
		ssboCPUMEM.vertexBuffer_B[i] = glm::vec4{	vertexBuffer[3 * i],
													vertexBuffer[(3 * i) + 1],
													vertexBuffer[(3 * i) + 2],
													0.0f };
	}

	for (int j = 0; j < 5522; ++j) {
		ssboCPUMEM.elementBuffer_A[j] = glm::uvec4{ elementBuffer[3 * j],
													elementBuffer[(3 * j) + 1],
													elementBuffer[(3 * j) + 2],
													0.0f };
		ssboCPUMEM.elementBuffer_B[j] = glm::uvec4{ elementBuffer[3 * j],
													elementBuffer[(3 * j) + 1],
													elementBuffer[(3 * j) + 2],
													0.0f };
	}

	// The two meshes move positive x direction:
	//  Mesh A moves 1 unit
	//  Mesh B moves 1.1 unit
	ssboCPUMEM.model_A =	glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ssboCPUMEM.model_B =	glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.0f)) *
							glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f)) *
							glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

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
	std::cout	<< "\nssbo BEFORE compute dispatch call:\n"
				<< "-----------------------------------------------\n";
	printSSBO();

	GLuint blockIndex = glGetProgramResourceIndex(computeProgram_id, GL_SHADER_STORAGE_BLOCK, "ssbo_data");

	if (blockIndex != GL_INVALID_INDEX) {
		GLuint ssbo_binding_point_index = 0;
		CHECKED_GL_CALL(glShaderStorageBlockBinding(computeProgram_id, blockIndex, ssbo_binding_point_index));
		CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboGPU_id));
	} else {
		std::cerr << "Warning: binding SSBO failed because it was not found on GPU.\n";
		exit(EXIT_FAILURE);
	}

	CHECKED_GL_CALL(glUseProgram(computeProgram_id));
	CHECKED_GL_CALL(glDispatchCompute(5522, 5522, 1));
	CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));

	// Wait for compute shader to finish writing to ssbo
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Copy data back to CPU MEM
	CHECKED_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGPU_id));
	GLvoid *dataGPUPtr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy(&ssboCPUMEM, dataGPUPtr, sizeof(SSBO));
	CHECKED_GL_CALL(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));

	// Unbind storage buffer and program objects
	CHECKED_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
	CHECKED_GL_CALL(glUseProgram(0));

	std::cout	<< "ssbo AFTER compute dispatch call:\n"
				<< "-----------------------------------------------\n";
	printSSBO();

	interpretComputedSSBO();
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
}