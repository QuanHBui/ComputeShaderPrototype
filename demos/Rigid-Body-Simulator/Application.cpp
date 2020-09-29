#include "Application.h"

#include <iomanip>
#include <iostream>
#include <glm/gtx/transform.hpp>
#include <string>

#include <tiny_obj_loader/tiny_obj_loader.h>

#include "ComputeProgram.h"
#include "GLSL.h"
#include "OpenGLUtils.h"
#include "Shape.h"
#include "stb_image.h"

// Physics stuff
#include "PrototypePhysicsEngine/P3BroadPhaseCollisionDetection.h"
#include "PrototypePhysicsEngine/P3NarrowPhaseCollisionDetection.h"

// UI stuff
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

#define EPSILON 0.0001f
#define COMPUTE_DEBUG false

static bool firstRun = true;
static bool verticalMove = false;
static bool moveLeft = false;
static bool moveRight = false;
static bool moveForward = false;
static bool moveBackward = false;

// imgui state(s)
static float f = 0.0f;

Application::~Application()
{
	std::cout << "Application has been safely destroyed.";
	mpWindowManager = nullptr;

	// imgui cleanup
	//ImGui_ImplOpenGL3_Shutdown();
	//ImGui_ImplGlfw_Shutdown();
	//ImGui::DestroyContext();
}

void Application::init()
{
	initGeom();
	initUI();
	initRenderProgram();
}

void Application::initGeom()
{
	//std::vector<tinyobj::shape_t> bunnyShapes, quadShapes;
	//std::vector<tinyobj::material_t> bunnyMaterials, quadMaterials;
	//std::string errStrBunny, errStrQuad;
	//bool bunnyLoadCheck, quadLoadCheck;

	//bunnyLoadCheck = tinyobj::LoadObj(bunnyShapes, bunnyMaterials, errStrBunny,
	//								  "../resources/models/bunny.obj");
	//quadLoadCheck = tinyobj::LoadObj(quadShapes, quadMaterials, errStrQuad,
	//								 "../resources/models/quad.obj");
	//if (!bunnyLoadCheck)
	//{
	//	std::cerr << errStrBunny << std::endl;
	//}
	//else if (!quadLoadCheck)
	//{
	//	std::cerr << errStrQuad << std::endl;
	//}
	//else
	//{
	//	mMeshContainer.emplace_back(std::make_unique<Shape>());
	//	mMeshContainer.back()->createShape(bunnyShapes.at(0));
	//	mMeshContainer.back()->init();
	//	mMeshContainer.back()->measure();
	//	mMeshContainer.back()->resize();

	//	mMeshContainer.emplace_back(std::make_unique<Shape>());
	//	mMeshContainer.back()->createShape(quadShapes.at(0));
	//	mMeshContainer.back()->init();
	//	mMeshContainer.back()->measure();
	//	mMeshContainer.back()->resize();
	//}

	//// Check for the size of the bunny mesh vertex buffer
	//printf("\nSize of bunny position buffer: %zd\nSize of bunny element buffer: %zd\n",
	//		mMeshContainer.at(0)->posBuf.size(), mMeshContainer.at(0)->eleBuf.size());
	//printf("\nSize of quad position buffer: %zd\nSize of quad element buffer: %zd\n\n",
	//		mMeshContainer.at(1)->posBuf.size(), mMeshContainer.at(1)->eleBuf.size());
	//fflush(stdout);

	//// Store VAO handle generated from Shape class
	//mVao = mMeshContainer.at(0)->getVaoID();
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
	ImGui_ImplGlfw_InitForOpenGL(mpWindowManager->getHandle(), true);
	ImGui_ImplOpenGL3_Init("#version 430");

	// Usually load font here, but let imgui use default front for now.
}

void Application::initRenderProgram()
{
	GLSL::checkVersion();

	// Set background color
	glClearColor(.12f, .34f, .56f, 1.0f);

	// Enabel z-buffer test
	glEnable(GL_DEPTH_TEST);

	//mpRenderProgram = std::make_unique<Program>();
	//mpRenderProgram->setVerbose(true);
	//mpRenderProgram->setShaderNames("../resources/shaders/vs.vert",
	//								"../resources/shaders/fs.frag");
	//mpRenderProgram->init();
	//mpRenderProgram->addAttribute("vertPos");
	//mpRenderProgram->addAttribute("vertNor");
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

// Bind SSBO to render program and draw
void Application::renderFrame()
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

	//// Bind UBO
	//glBindBuffer(GL_UNIFORM_BUFFER, mUboGpuID);
	//glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUboGpuID);

	//// Bind render program
	//mpRenderProgram->bind();
	//// Send model matrix
	//glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "model"),
	//				   1, GL_FALSE, glm::value_ptr(mUboCpuMem.model_A));
	//mMeshContainer.at(0)->draw(mpRenderProgram, mSsboGpuID[1]);		// Draw bunny

	//glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "model"),
	//				   1, GL_FALSE, glm::value_ptr(mUboCpuMem.model_B));
	//mMeshContainer.at(1)->draw(mpRenderProgram, 0u);				// Draw quad

	//// When done, unbind UBO
	//glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//mpRenderProgram->unbind();
}

void Application::renderUI(double dt)
{
	// Start imgui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Create a window called "Hello, world!" and append into it
	ImGui::Begin("Hello, world!");

	// Display some text
	ImGui::Text("FPS: %.1f | Frame time: %.3f ms", 1.0f / dt, dt);
	ImGui::Checkbox("A checkbox", &verticalMove);

	// Slider with float values
	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

	ImGui::End();

	// Actual rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::update(float dt)
{
}