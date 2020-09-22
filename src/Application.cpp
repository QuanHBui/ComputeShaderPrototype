#include "Application.h"

#include <iomanip>
#include <iostream>
#include <glm/gtx/transform.hpp>
#include <string>

#include <tiny_obj_loader/tiny_obj_loader.h>

#include "ComputeProgram.h"
#include "GLSL.h"
#include "OpenGLUtils.h"
#include "PrototypePhysicsEngine/P3BroadPhaseCollisionDetection.h"
#include "PrototypePhysicsEngine/P3NarrowPhaseCollisionDetection.h"
#include "Shape.h"
#include "stb_image.h"

// UI stuff
#include "imgui/imgui.h"

#define EPSILON 0.0001f
#define COMPUTE_DEBUG false

static bool firstRun = true;
static bool verticalMove = false;
static bool moveLeft = false;
static bool moveRight = false;
static bool moveForward = false;
static bool moveBackward = false;

void Application::printSsbo()
{
	const glm::vec4 *positionBuffer_A = &mSsboCpuMem.positionBuffer_A[0];
	const glm::vec4 *positionBuffer_B = &mSsboCpuMem.positionBuffer_B[0];
	const glm::uvec4 *elementBuffer_A = &mSsboCpuMem.elementBuffer_A[0];
	const glm::uvec4 *elementBuffer_B = &mSsboCpuMem.elementBuffer_B[0];

	const glm::mat4 &model_A = mUboCpuMem.model_A;
	const glm::mat4 &model_B = mUboCpuMem.model_B;

	printf("model_A:\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
			model_A[0][1], model_A[1][1], model_A[2][1], model_A[3][1],
			model_A[0][0], model_A[1][0], model_A[2][0], model_A[3][0],
			model_A[0][2], model_A[1][2], model_A[2][2], model_A[3][2],
			model_A[0][3], model_A[1][3], model_A[2][3], model_A[3][3]);

	printf("model_B:\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
			model_B[0][0], model_B[1][0], model_B[2][0], model_B[3][0],
			model_B[0][1], model_B[1][1], model_B[2][1], model_B[3][1],
			model_B[0][2], model_B[1][2], model_B[2][2], model_B[3][2],
			model_B[0][3], model_B[1][3], model_B[2][3], model_B[3][3]);

	// Print out nicely the first 5 elements of the ssbo
	for (int i = 0; i < 5; ++i)
	{
		std::cout << "positionBuffer_A: "	<< std::setprecision(5)
											<< positionBuffer_A[i].x << ", "
											<< positionBuffer_A[i].y << ", "
											<< positionBuffer_A[i].z << ", "
											<< positionBuffer_A[i].w << '\n';
		std::cout << "positionBuffer_B: "	<< std::setprecision(5)
											<< positionBuffer_B[i].x << ", "
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

void Application::printColorSsbo()
{
	const glm::vec4 *colorBuffer_A = &mColorOutSsbo.colorBuffer_A[0];
	const glm::vec4 *colorBuffer_B = &mColorOutSsbo.colorBuffer_B[0];

	for (int i = 0; i < 7; ++i)
	{
		std::cout << "localColorBuffer_A: "	<< std::setprecision(5)
											<< colorBuffer_A[i].r << ", "
											<< colorBuffer_A[i].g << ", "
											<< colorBuffer_A[i].b << ", "
											<< colorBuffer_A[i].q << '\n';
		std::cout << "localColorBuffer_B: "	<< std::setprecision(5)
											<< colorBuffer_B[i].r << ", "
											<< colorBuffer_B[i].g << ", "
											<< colorBuffer_B[i].b << ", "
											<< colorBuffer_B[i].q << "\n\n";
	}
	std::cout << std::endl;
}

void Application::interpretComputedSsbo()
{
	const glm::uvec4 *elementBuffer_A = &mSsboCpuMem.elementBuffer_A[0];
	const glm::uvec4 *elementBuffer_B = &mSsboCpuMem.elementBuffer_B[0];

	// 5522 is the number of triangles in the bunny mesh
	// Loop through the element buffer and if w component is 1, that
	//  triangle has collided

	// Two loops but this is for better cache locality. Computers love
	//  tightly packed arrays
	std::cout << "In mesh A, collided triangles are:\n";
	for (int i = 0; i < 5522; ++i)
	{
		if (elementBuffer_A[i].w == 1u)
		{
			std::cout << i;

			if (i != 5521) std::cout << ", ";
		}
	}
	std::cout << "\n\n";

	std::cout << "In mesh B, collided triangles are:\n";
	for (int j = 0; j < 5522; ++j)
	{
		if (elementBuffer_B[j].w == 1u)
		{
			std::cout << j;

			if (j != 5521) std::cout << ", ";
		}
	}
	std::cout << '\n' << std::endl;
}

Application::~Application()
{
	std::cout << "Application is safely destroyed.";
	mpWindowManager = nullptr;

	if (mSsboGpuID)
		CHECKED_GL_CALL(glDeleteBuffers(2, mSsboGpuID));
	if (mUboGpuID)
		CHECKED_GL_CALL(glDeleteBuffers(1, &mUboGpuID));

	if (!mComputeProgramIDContainer.empty())
	{
		for (GLuint computeProgramID : mComputeProgramIDContainer)
			CHECKED_GL_CALL(glDeleteProgram(computeProgramID));
	}
}

void Application::init()
{
	initGeom();
	initUI();
	initCpuBuffers();
	initGpuBuffers();
	initComputePrograms();
	initRenderProgram();
}

void Application::initGeom()
{
	std::vector<tinyobj::shape_t> bunnyShapes, quadShapes;
	std::vector<tinyobj::material_t> bunnyMaterials, quadMaterials;
	std::string errStrBunny, errStrQuad;
	bool bunnyLoadCheck, quadLoadCheck;

	bunnyLoadCheck = tinyobj::LoadObj(bunnyShapes, bunnyMaterials, errStrBunny,
									  "../resources/models/bunny.obj");
	quadLoadCheck = tinyobj::LoadObj(quadShapes, quadMaterials, errStrQuad,
									 "../resources/models/quad.obj");
	if (!bunnyLoadCheck)
	{
		std::cerr << errStrBunny << std::endl;
	}
	else if (!quadLoadCheck)
	{
		std::cerr << errStrQuad << std::endl;
	}
	else
	{
		mMeshContainer.emplace_back(std::make_unique<Shape>());
		mMeshContainer.back()->createShape(bunnyShapes.at(0));
		mMeshContainer.back()->init();
		mMeshContainer.back()->measure();
		mMeshContainer.back()->resize();

		mMeshContainer.emplace_back(std::make_unique<Shape>());
		mMeshContainer.back()->createShape(quadShapes.at(0));
		mMeshContainer.back()->init();
		mMeshContainer.back()->measure();
		mMeshContainer.back()->resize();
	}

	// Check for the size of the bunny mesh vertex buffer
	printf("\nSize of bunny position buffer: %zd\nSize of bunny element buffer: %zd\n",
			mMeshContainer.at(0)->posBuf.size(), mMeshContainer.at(0)->eleBuf.size());
	printf("\nSize of quad position buffer: %zd\nSize of quad element buffer: %zd\n\n",
			mMeshContainer.at(1)->posBuf.size(), mMeshContainer.at(1)->eleBuf.size());
	fflush(stdout);

	// Store VAO handle generated from Shape class
	mVao = mMeshContainer.at(0)->getVaoID();
}

void Application::initUI()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	//ImGui_ImplGlfw_InitForOpenGL(window, true);
	//ImGui_ImplOpenGL3_Init("#version 430");


	//ImGui::Text("Hello, world %d", 123);
	//if (ImGui::Button("Save"))
	//{

	//}
}

void Application::initCpuBuffers()
{
	// Initialize SSBO with the position and element buffers from loading mesh obj, pre-transformed
	{
		const std::vector<float> &positionBuffer_A = mMeshContainer.at(0)->posBuf;
		const std::vector<unsigned int> &elementBuffer_A = mMeshContainer.at(0)->eleBuf;

		const std::vector<float> &positionBuffer_B = mMeshContainer.at(1)->posBuf;
		const std::vector<unsigned int> &elementBuffer_B = mMeshContainer.at(1)->eleBuf;

		// Multiple loops for better cache locality. It's actually not as inefficient as you might think
		for (int i = 0; i < 2763; ++i)
		{
			mSsboCpuMem.positionBuffer_A[i] = glm::vec4(positionBuffer_A[3 * i],
														positionBuffer_A[(3 * i) + 1],
														positionBuffer_A[(3 * i) + 2],
														1.0f);
		}

		for (int i = 0; i < 2763; ++i)
		{
			if (i < 4)
			{
				mSsboCpuMem.positionBuffer_B[i] = glm::vec4(positionBuffer_B[3 * i],
															positionBuffer_B[(3 * i) + 1],
															positionBuffer_B[(3 * i) + 2],
															1.0f);
			}
			else
			{
				mSsboCpuMem.positionBuffer_B[i] = glm::vec4(0.0f);
			}
		}

		for (int j = 0; j < 5522; ++j)
		{
			mSsboCpuMem.elementBuffer_A[j] = glm::uvec4(elementBuffer_A[3 * j],
														elementBuffer_A[(3 * j) + 1],
														elementBuffer_A[(3 * j) + 2],
														0u);
		}

		for (int j = 0; j < 5522; ++j)
		{
			if (j < 2)
			{
				mSsboCpuMem.elementBuffer_B[j] = glm::uvec4(elementBuffer_B[3 * j],
															elementBuffer_B[(3 * j) + 1],
															elementBuffer_B[(3 * j) + 2],
															0u);
			}
			else
			{
				mSsboCpuMem.elementBuffer_B[j] = glm::uvec4(0u);
			}
		}
	}

	// Prep uniform data on the CPU
	mUboCpuMem.model_A = glm::translate(glm::vec3(1.0f, 0.0f, -1.0f));
	mUboCpuMem.model_B = glm::translate(glm::vec3(-1.5f, 0.75f, -1.0f))
						 * glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	mUboCpuMem.view = mFlyCamera.getViewMatrix();
	mUboCpuMem.projection = mFlyCamera.getProjectionMatrix();
}

void Application::initGpuBuffers()
{
	getComputeGroupInfo();
	getUboInfo();
	std::cout << '\n';

	// Allocate 2 SSBOs on GPU: 1 for input, and 1 for output
	glGenBuffers(2, mSsboGpuID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboGpuID[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(mSsboCpuMem), &mSsboCpuMem, GL_STATIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboGpuID[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(mColorOutSsbo), nullptr, GL_STREAM_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind ssbo

	// Allocate an UBO
	glGenBuffers(1, &mUboGpuID);
	glBindBuffer(GL_UNIFORM_BUFFER, mUboGpuID);
	glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(glm::mat4), nullptr, GL_STREAM_READ);

	// Send uniform data to GPU
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Ubo), &mUboCpuMem);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Application::initRenderProgram()
{
	GLSL::checkVersion();

	// Set background color
	glClearColor(.12f, .34f, .56f, 1.0f);
	// Enabel z-buffer test
	glEnable(GL_DEPTH_TEST);

	mpRenderProgram = std::make_unique<Program>();
	mpRenderProgram->setVerbose(true);
	mpRenderProgram->setShaderNames("../resources/shaders/vs.vert",
									"../resources/shaders/fs.frag");
	mpRenderProgram->init();
	mpRenderProgram->addAttribute("vertPos");
	mpRenderProgram->addAttribute("vertNor");
}

// General OGL initialization - set OGL state here
void Application::initComputePrograms()
{
	GLSL::checkVersion();

	GLuint computeProgramID = createComputeProgram("../resources/shaders/triTriTest.comp");
	mComputeProgramIDContainer.push_back(computeProgramID);

	// Set up the broad phase collision detection
	P3OpenGLComputeBroadPhaseCreateInfo createInfo;
	P3OpenGLComputeBroadPhase broadPhase(&createInfo);
}

void Application::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		if (glfwGetInputMode(mpWindowManager->getHandle(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		{
			glfwSetInputMode(mpWindowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else
		{
			glfwSetInputMode(mpWindowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		verticalMove = !verticalMove;
	}
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		moveLeft = true;
	}
	if (key == GLFW_KEY_A && action == GLFW_RELEASE)
	{
		moveLeft = false;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		moveRight = true;
	}
	if (key == GLFW_KEY_D && action == GLFW_RELEASE)
	{
		moveRight = false;
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		moveForward = true;
	}
	if (key == GLFW_KEY_W && action == GLFW_RELEASE)
	{
		moveForward = false;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		moveBackward = true;
	}
	if (key == GLFW_KEY_S && action == GLFW_RELEASE)
	{
		moveBackward = false;
	}
}

/**
 *	cursorCallback
 *	Every time the cursor move, this callback got invoked.
 */
void Application::cursorCallback(GLFWwindow *window, double xPos, double yPos)
{
	// Calculate deltaX and deltaY, then send to moveCameraView function
	//  camera should ONLY move after the initial cursor focus.
	if (mIsFirstCursorFocus)
	{
		mLastCursorPosX = xPos;
		mLastCursorPosY = yPos;
		mIsFirstCursorFocus = false;
	}
	mCursorPosDeltaX = static_cast<float>(xPos - mLastCursorPosX);
	mCursorPosDeltaY = static_cast<float>(mLastCursorPosY - yPos);	// Has to be reversed cuz math
	mFlyCamera.moveView(mCursorPosDeltaX, mCursorPosDeltaY);

	mLastCursorPosX = xPos;
	mLastCursorPosY = yPos;
}

// Bind SSBO to compute program and dispatch work group
void Application::computeOnGpu()
{
	static unsigned int frameCount = 5u;

	if (COMPUTE_DEBUG && frameCount)
	{
		std::cout << "\nColor SSBO BEFORE compute dispatch call:\n"
				  << "-----------------------------------------------\n";
		printColorSsbo();
	}

	// Prob don't have to set binding points every time. Should check ===================
	// Make sure this matches up with the binding point 1, which is set in the shader
	CHECKED_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboGpuID[0]));
	CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1u, mSsboGpuID[0]));

	CHECKED_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboGpuID[1]));
	CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2u, mSsboGpuID[1]));

	// Bind UBO to binding point 0
	CHECKED_GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, mUboGpuID));
	CHECKED_GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, 0u, mUboGpuID));
	//===================================================================================

	CHECKED_GL_CALL(glUseProgram(mComputeProgramIDContainer[0]));
	CHECKED_GL_CALL(glDispatchCompute(5522, 1, 1));

	// Wait for compute shader to finish writing to SSBO before reading from SSBO
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	if (COMPUTE_DEBUG && frameCount)
	{
	// if (true) {
		// Copy data back to CPU Memory
		GLvoid *dataGPUPtr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		memcpy(&mColorOutSsbo, dataGPUPtr, sizeof(mColorOutSsbo));
		CHECKED_GL_CALL(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));

		--frameCount;
	}

	// I guess this trying to unbind the CPU ssbo from the binding point 1
	CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1u, 0u));
	CHECKED_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2u, 0u));
	// Do I want to unbind CPU UBO from binding point 0 here??
	CHECKED_GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, 0u, 0u));

	// Unbind storage and uniform buffers and program objects
	CHECKED_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	CHECKED_GL_CALL(glUseProgram(0));

	if (COMPUTE_DEBUG && firstRun)
	{
	// if (true) {
		std::cout << "Color SSBO AFTER compute dispatch call:\n"
				  << "-----------------------------------------------\n";
		printColorSsbo();

		// interpretComputedSsbo();

		firstRun = false;
	}
}

// TODO: Multithread?
void Application::computeOnCpu()
{
	static unsigned int frameCount = 5u;

	if (COMPUTE_DEBUG && frameCount)
	{
		std::cout << "\nColor SSBO BEFORE compute dispatch call:\n"
				  << "-----------------------------------------------\n";
		printColorSsbo();
	}

	// TODO: This is just wasting memory. Please fix!
	glm::mat4 const &model_A = mUboCpuMem.model_A;
	glm::mat4 const &model_B = mUboCpuMem.model_B;
	glm::vec4 const (&rPositionBuffer_A)[2763] = mSsboCpuMem.positionBuffer_A;
	glm::vec4 const (&rPositionBuffer_B)[2763] = mSsboCpuMem.positionBuffer_B;
	glm::uvec4 const (&rElementBuffer_A)[5522] = mSsboCpuMem.elementBuffer_A;
	glm::uvec4 const (&rElementBuffer_B)[5522] = mSsboCpuMem.elementBuffer_B;
	glm::vec4 (&colorBuffer_A)[2763] = mColorOutSsbo.colorBuffer_A;
	glm::vec4 (&colorBuffer_B)[2763] = mColorOutSsbo.colorBuffer_B;
	glm::vec3 v0(0.0f), v1(0.0f), v2(0.0f), u0(0.0f), u1(0.0f), u2(0.0f);
	glm::uvec3 tri_A(0), tri_B(0);
	bool isColliding = false;

	for (unsigned int elementBufferIdx_B = 0; elementBufferIdx_B < 2; ++elementBufferIdx_B)
	{
		tri_B = rElementBuffer_B[elementBufferIdx_B].xyz();

		// Prep the vertices: mapping them to world space
		u0 = (model_B * rPositionBuffer_B[tri_B.x]).xyz();
		u1 = (model_B * rPositionBuffer_B[tri_B.y]).xyz();
		u2 = (model_B * rPositionBuffer_B[tri_B.z]).xyz();

		for (unsigned int elementBufferIdx_A = 0; elementBufferIdx_A < 5522; ++elementBufferIdx_A)
		{
			tri_A = rElementBuffer_A[elementBufferIdx_A].xyz();

			// Prep the vertices: mapping them to world space
			v0 = (model_A * rPositionBuffer_A[tri_A.x]).xyz();
			v1 = (model_A * rPositionBuffer_A[tri_A.y]).xyz();
			v2 = (model_A * rPositionBuffer_A[tri_A.z]).xyz();

			// Query the test
			isColliding = fastTriTriIntersect3DTest(v0, v1, v2, u0, u1, u2);

			// Output the results to CPU memory
			if (isColliding)
			{
				colorBuffer_A[tri_A.x] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
				colorBuffer_A[tri_A.y] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
				colorBuffer_A[tri_A.z] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

				colorBuffer_B[tri_B.x] = glm::vec4(3.0f, 0.0f, 0.0f, 1.0f);
				colorBuffer_B[tri_B.y] = glm::vec4(3.0f, 0.0f, 0.0f, 1.0f);
				colorBuffer_B[tri_B.z] = glm::vec4(3.0f, 0.0f, 0.0f, 1.0f);
			}
			else
			{
				colorBuffer_A[tri_A.x] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				colorBuffer_A[tri_A.y] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				colorBuffer_A[tri_A.z] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

				colorBuffer_B[tri_B.x] = glm::vec4(2.0f, 0.0f, 0.0f, 1.0f);
				colorBuffer_B[tri_B.y] = glm::vec4(2.0f, 0.0f, 0.0f, 1.0f);
				colorBuffer_B[tri_B.z] = glm::vec4(2.0f, 0.0f, 0.0f, 1.0f);
			}
		}
	}

	// Transfer results to GPU buffers
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSsboGpuID[1]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ColorOutSsbo), &mColorOutSsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// Bind SSBO to render program and draw
void Application::render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(mpWindowManager->getHandle(), &width, &height);

	// Prevent assertion when minimize the window
	if (!width && !height) return;

	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float aspect = width/(float)height;

	// Bind UBO
	glBindBuffer(GL_UNIFORM_BUFFER, mUboGpuID);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUboGpuID);

	// Bind render program
	mpRenderProgram->bind();
	// Send model matrix
	glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "model"),
					   1, GL_FALSE, glm::value_ptr(mUboCpuMem.model_A));
	mMeshContainer.at(0)->draw(mpRenderProgram, mSsboGpuID[1]);		// Draw bunny

	glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "model"),
					   1, GL_FALSE, glm::value_ptr(mUboCpuMem.model_B));
	mMeshContainer.at(1)->draw(mpRenderProgram, 0u);				// Draw quad

	// When done, unbind UBO
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	mpRenderProgram->unbind();
}

void Application::update(float dt)
{
	updateCpuBuffers(dt);
	updateGpuBuffers();
}

void Application::updateCpuBuffers(float dt)
{
	// Update translation of mesh B. TODO: Might want to use dt here.
	if (verticalMove)
	{
		mUboCpuMem.model_B = glm::translate(glm::vec3(0.9f, 2.0f * sinf(0.5f * (float)glfwGetTime()), -1.0f)) *
							 glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else
	{
		mUboCpuMem.model_B = glm::translate(glm::vec3(-1.5f + 3.5f * sinf(0.4f * (float)glfwGetTime()), 0.75f, -1.0f)) *
							 glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}

	// Check for camera movement flag to update view matrix
	if (moveForward)
	{
		mFlyCamera.movePosition(mFlyCamera.FORWARD, dt);
	}
	if (moveBackward)
	{
		mFlyCamera.movePosition(mFlyCamera.BACKWARD, dt);
	}
	if (moveLeft)
	{
		mFlyCamera.movePosition(mFlyCamera.LEFT, dt);
	}
	if (moveRight)
	{
		mFlyCamera.movePosition(mFlyCamera.RIGHT, dt);
	}

	// Both view and project matrices are controlled by the camera.
	mUboCpuMem.projection = mFlyCamera.getProjectionMatrix();
	mUboCpuMem.view = mFlyCamera.getViewMatrix();
}

void Application::updateGpuBuffers()
{
	glBindBuffer(GL_UNIFORM_BUFFER, mUboGpuID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Ubo), &mUboCpuMem);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}