#include "Application.h"

#include <iostream>
#include <glm/gtx/transform.hpp>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "GLSL.h"
#include "Shape.h"
#include "stb_image.h"

#define EPSILON 0.0001f
#define COMPUTE_DEBUG true

static bool firstRun = true;

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

	if (ssboGPU_id)
		CHECKED_GL_CALL(glDeleteBuffers(2, ssboGPU_id));
	if (computeUBOGPU_id)
		CHECKED_GL_CALL(glDeleteBuffers(1, &computeUBOGPU_id));
	if (renderUBOGPU_id)
		CHECKED_GL_CALL(glDeleteBuffers(1, &renderUBOGPU_id));
	if (computeProgram_id)
		CHECKED_GL_CALL(glDeleteProgram(computeProgram_id));
}

void Application::printSSBO()
{
	const glm::vec4 *positionBuffer_A = &ssboCPUMEM.positionBuffer_A[0];
	const glm::vec4 *positionBuffer_B = &ssboCPUMEM.positionBuffer_B[0];
	const glm::uvec4 *elementBuffer_A = &ssboCPUMEM.elementBuffer_A[0];
	const glm::uvec4 *elementBuffer_B = &ssboCPUMEM.elementBuffer_B[0];

	const glm::mat4 &model_A = uboCPUMEM.model_A;
	const glm::mat4 &model_B = uboCPUMEM.model_B;

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
		std::cout << "positionBuffer_A: "	<< positionBuffer_A[i].x << ", "
										<< positionBuffer_A[i].y << ", "
										<< positionBuffer_A[i].z << ", "
										<< positionBuffer_A[i].w << '\n';
		std::cout << "positionBuffer_B: "	<< positionBuffer_B[i].x << ", "
										<< positionBuffer_B[i].y << ", "
										<< positionBuffer_B[i].z << ", "
										<< positionBuffer_B[i].w << '\n';
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

void Application::printColorSSBO()
{
	const glm::vec4 *localColorBuffer_A = &colorOutSSBO.colorBuffer_A[0];
	const glm::vec4 *localColorBuffer_B = &colorOutSSBO.colorBuffer_B[0];

	// Print out nicely the first 5 elements of the ssbo
	for (int i = 0; i < 5; ++i) {
		std::cout << "localColorBuffer_A: "	<< localColorBuffer_A[i].r << ", "
											<< localColorBuffer_A[i].g << ", "
											<< localColorBuffer_A[i].b << ", "
											<< localColorBuffer_A[i].q << '\n';
		std::cout << "localColorBuffer_B: "	<< localColorBuffer_B[i].r << ", "
											<< localColorBuffer_B[i].g << ", "
											<< localColorBuffer_B[i].b << ", "
											<< localColorBuffer_B[i].q << "\n\n";
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
	std::vector<tinyobj::shape_t> bunnyShapes, quadShapes;
	std::vector<tinyobj::material_t> bunnyMaterials, quadMaterials;
	std::string errStrBunny, errStrQuad;
	bool bunnyLoadCheck, quadLoadCheck;

	bunnyLoadCheck = tinyobj::LoadObj(	bunnyShapes, bunnyMaterials,
										errStrBunny, "../resources/models/bunny.obj");
	quadLoadCheck = tinyobj::LoadObj(	quadShapes, quadMaterials, errStrQuad,
										"../resources/models/quad.obj");
	if (!bunnyLoadCheck)
		std::cerr << errStrBunny << std::endl;
	else if (!quadLoadCheck)
		std::cerr << errStrQuad << std::endl;
	else {
		meshContainer.emplace_back(std::make_unique<Shape>());
		meshContainer.back()->createShape(bunnyShapes.at(0));
		meshContainer.back()->init();
		meshContainer.back()->measure();
		meshContainer.back()->resize();

		meshContainer.emplace_back(std::make_unique<Shape>());
		meshContainer.back()->createShape(quadShapes.at(0));
		meshContainer.back()->init();
		meshContainer.back()->measure();
		meshContainer.back()->resize();
	}

	// Check for the size of the bunny mesh vertex buffer
	printf("\nSize of bunny vertex buffer: %zd\nSize of bunny element buffer: %zd\n",
			meshContainer.at(0)->posBuf.size(), meshContainer.at(0)->eleBuf.size());
	printf("\nSize of quad vertex buffer: %zd\nSize of quad element buffer: %zd\n\n",
			meshContainer.at(1)->posBuf.size(), meshContainer.at(1)->eleBuf.size());
	fflush(stdout);

	// Store VAO handle generated from Shape class
	VAO = meshContainer.at(0)->vaoID;
}

// Initialize SSBO with the position and element buffers from loading mesh obj, pre-transformed
void Application::initSSBO()
{
	const std::vector<float> &localPositionBuffer_A = meshContainer.at(0)->posBuf;
	const std::vector<unsigned int> &localElementBuffer_A = meshContainer.at(0)->eleBuf;

	const std::vector<float> &localPositionBuffer_B = meshContainer.at(1)->posBuf;
	const std::vector<unsigned int> &localElementBuffer_B = meshContainer.at(1)->eleBuf;

	// Multiple loops for better cache locality. It's actually not as inefficient as you might think
	for (int i = 0; i < 2763; ++i) {
		ssboCPUMEM.positionBuffer_A[i] = glm::vec4{	localPositionBuffer_A[3 * i],
													localPositionBuffer_A[(3 * i) + 1],
													localPositionBuffer_A[(3 * i) + 2],
													0.0f };
	}

	for (int i = 0; i < 2763; ++i) {
		if (i < 4) {
			ssboCPUMEM.positionBuffer_B[i] = glm::vec4{	localPositionBuffer_B[3 * i],
														localPositionBuffer_B[(3 * i) + 1],
														localPositionBuffer_B[(3 * i) + 2],
														0.0f };
		} else {
			ssboCPUMEM.positionBuffer_B[i] = glm::vec4{ 0.0f };
		}
	}

	for (int j = 0; j < 5522; ++j) {
		ssboCPUMEM.elementBuffer_A[j] = glm::uvec4{ localElementBuffer_A[3 * j],
													localElementBuffer_A[(3 * j) + 1],
													localElementBuffer_A[(3 * j) + 2],
													0u };
	}

	for (int j = 0; j < 5522; ++j) {
		if (j < 2) {
			ssboCPUMEM.elementBuffer_B[j] = glm::uvec4{ localElementBuffer_B[3 * j],
														localElementBuffer_B[(3 * j) + 1],
														localElementBuffer_B[(3 * j) + 2],
														0u };
		} else {
			ssboCPUMEM.elementBuffer_B[j] = glm::uvec4{ 0u };
		}
	}

	// Allocate 2 SSBOs on GPU: 1 for input, and 1 for output
	glGenBuffers(2, ssboGPU_id);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGPU_id[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssboCPUMEM), &ssboCPUMEM, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGPU_id[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(colorOutSSBO), nullptr, GL_STREAM_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind ssbo

	// Allocate an UBO
	glGenBuffers(1, &computeUBOGPU_id);
	glBindBuffer(GL_UNIFORM_BUFFER, computeUBOGPU_id);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STREAM_READ);

	// Prep and send data to GPU
	uboCPUMEM.model_A = glm::translate(glm::vec3(1.0f, 0.0f, -1.0));
	uboCPUMEM.model_B =	glm::translate(glm::vec3(-2.0f, 0.75f, -1.0f)) *
						glm::rotate(glm::radians(0.0f), glm::vec3(1.0f));

	// Fill the buffers with data
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(uboCPUMEM.model_A));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(uboCPUMEM.model_B));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Application::initRenderProgram()
{
	GLSL::checkVersion();

	// Set background color
	glClearColor(.12f, .34f, .56f, 1.0f);
	// Enabel z-buffer test
	glEnable(GL_DEPTH_TEST);

	renderProgramPtr = std::make_unique<Program>();
	renderProgramPtr->setVerbose(true);
	renderProgramPtr->setShaderNames("../resources/shaders/vs.glsl",
									"../resources/shaders/fs.glsl");
	renderProgramPtr->init();
	renderProgramPtr->addAttribute("vertPos");
	renderProgramPtr->addAttribute("vertNor");

	glGenBuffers(1, &renderUBOGPU_id);
	glBindBuffer(GL_UNIFORM_BUFFER, renderUBOGPU_id);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);	// Unbind UBO
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
	if (COMPUTE_DEBUG && firstRun) {
		std::cout	<< "\nColor SSBO BEFORE compute dispatch call:\n"
					<< "-----------------------------------------------\n";
		printColorSSBO();
	}

	// Prob don't have to set binding points every time. Should check ===================
	// Make sure this matches up with the binding point 1, which is set in the shader
	CHECKED_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGPU_id[0]));
	CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1u, ssboGPU_id[0]));

	CHECKED_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGPU_id[1]));
	CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2u, ssboGPU_id[1]));

	// Bind UBO to binding point 0
	CHECKED_GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, computeUBOGPU_id));
	CHECKED_GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, 0u, computeUBOGPU_id));
	//===================================================================================

	CHECKED_GL_CALL(glUseProgram(computeProgram_id));
	CHECKED_GL_CALL(glDispatchCompute(5522, 2, 1));

	// Wait for compute shader to finish writing to ssbo before reading from ssbo
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	if (COMPUTE_DEBUG && firstRun) {
		// Copy data back to CPU MEM
		GLvoid *dataGPUPtr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		memcpy(&colorOutSSBO, dataGPUPtr, sizeof(colorOutSSBO));
		CHECKED_GL_CALL(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
	}

	// I guess this trying to unbind the CPU ssbo from the binding point 1
	CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1u, 0u));
	// Do I want to unbind CPU UBO from binding point 0 here??
	CHECKED_GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, 0u, 0u));

	// Unbind storage and uniform buffers and program objects
	CHECKED_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	CHECKED_GL_CALL(glUseProgram(0));

	if (COMPUTE_DEBUG && firstRun) {
		std::cout	<< "Color SSBO AFTER compute dispatch call:\n"
					<< "-----------------------------------------------\n";
		printColorSSBO();
	// interpretComputedSSBO();

		firstRun = false;
	}
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

	uboCPUMEM.projection = glm::perspective(45.0f, aspect, 0.01f, 150.0f);
	uboCPUMEM.view = glm::lookAt(	glm::vec3{ 0.0f, 3.0f, 3.0f },
									glm::vec3{ 0.0f, -0.5f, -2.0f },
									glm::vec3{ 0.0f, 1.0f, 0.0f });

	// Bind rendering UBO
	glBindBuffer(GL_UNIFORM_BUFFER, renderUBOGPU_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, renderUBOGPU_id);
	// Send data to UBO
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
					glm::value_ptr(uboCPUMEM.projection));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
					glm::value_ptr(uboCPUMEM.view));

	// Bind render program
	renderProgramPtr->bind();
	// Send model matrix
	glUniformMatrix4fv(glGetUniformLocation(renderProgramPtr->getPID(), "model"),
						1, GL_FALSE, glm::value_ptr(uboCPUMEM.model_A));
	meshContainer.at(0)->draw(renderProgramPtr, ssboGPU_id[1]);		// Draw bunny

	glUniformMatrix4fv(glGetUniformLocation(renderProgramPtr->getPID(), "model"),
						1, GL_FALSE, glm::value_ptr(uboCPUMEM.model_B));
	meshContainer.at(1)->draw(renderProgramPtr, 0u);				// Draw quad

	// When done, unbind UBO
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	renderProgramPtr->unbind();
}

void Application::update()
{
	static bool stop = false;
	// Update translation of bunny 2
	if (!stop && abs(-2.0 + 3.0f * sinf(0.3f * (float)glfwGetTime()) - 0.9f) > EPSILON) {
		uboCPUMEM.model_B =	glm::translate(glm::vec3( -2.0f + 3.0f * sinf(0.3f * (float)glfwGetTime()), 0.75f, -1.0f)) *
							glm::rotate(glm::radians(0.0f), glm::vec3(1.0f));
	} else {
		stop = true;
	}

	// Update UBO data
	glBindBuffer(GL_UNIFORM_BUFFER, computeUBOGPU_id);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(uboCPUMEM.model_A));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(uboCPUMEM.model_B));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}