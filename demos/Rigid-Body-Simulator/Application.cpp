#include "Application.h"

#include <iomanip>
#include <iostream>
#include <string>

#include <glm/gtx/transform.hpp>
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "PrototypePhysicsEngine/ComputeProgram.h"
#include "GLSL.h"
#include "OpenGLUtils.h"
#include "Shape.h"

// Physics stuff
#include "PrototypePhysicsEngine/P3DynamicsWorld.h"

// UI stuff
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

#define EPSILON 0.0001f
#define COMPUTE_DEBUG false

static bool firstRun     = true;
static bool moveLeft     = false;
static bool moveRight    = false;
static bool moveForward  = false;
static bool moveBackward = false;
static bool moveUpward   = false;
static bool moveDownward = false;

// imgui state(s)
static bool showHitBoxVerts = true;
static bool showContactPts  = true;
static bool allowToAdd = false;

Application::~Application()
{
	std::cout << "Application has been safely destroyed.";
	mpWindowManager = nullptr;

	// imgui cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Application::init()
{
	std::cout << '\n';

	oglutils::getComputeShaderInfo();
	oglutils::getUboInfo();

	std::cout << '\n';

	initRenderSystem();
	initPhysicsWorld(Demo::ROTATIONAL_TEST);
	initUI();
}

void Application::initRenderSystem()
{
	int width, height;
	glfwGetFramebufferSize(mpWindowManager->getHandle(), &width, &height);
	mRenderSystem.init(width, height);

	// Move camera a bit closer. This depends on which demo is showing.
	mFlyCamera.setPosition(glm::vec3(0.0f, 2.0f, 50.0f));
	mFlyCamera.updateViewMatrix();

	mRenderSystem.setView(mFlyCamera.getViewMatrix());
	mRenderSystem.setProjection(mFlyCamera.getProjectionMatrix());
}

void Application::initPhysicsWorld(Demo demo)
{
	mPhysicsWorld.init();

	switch (demo)
	{
	case Demo::BOWLING_GAME:
		mPhysicsWorld.bowlingGameDemo();
		mRenderSystem.registerMeshForBody(RenderSystem::MeshKey::BOWLING_PIN, mPhysicsWorld.getOccupancy());
		break;

	case Demo::MULTIPLE_BOXES:
		mPhysicsWorld.multipleBoxesDemo();
		mRenderSystem.registerMeshForBody(RenderSystem::MeshKey::CUBE, mPhysicsWorld.getOccupancy());
		break;

	case Demo::GRAVITY_TEST:
		// Stack 2 unit cubes on top of each other. Same mass.
		mPhysicsWorld.addRigidBody(1.0f, glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f));
		mPhysicsWorld.addStaticBody(glm::vec3(0.0f, -2.0f, 5.0f));
		mRenderSystem.registerMeshForBody(RenderSystem::MeshKey::CUBE, mPhysicsWorld.getOccupancy());
		break;

	case Demo::ROTATIONAL_TEST:
		mPhysicsWorld.addRigidBody(1.0f, glm::vec3( 0.5f, 6.0f, 5.0f), glm::vec3(-10.0f, 0.0f, 0.0f));

		for (int i = 0; i < 3; ++i)
		{
			mPhysicsWorld.addRigidBody(-1.5f, glm::vec3(2.5f + float(i) * 3.0f, 2.0f, 5.0f), glm::vec3(-10.0f, 0.0f, 0.0f));
		}

		mPhysicsWorld.addStaticBody(glm::vec3( 2.0f, -2.0f, 5.0f));
		mPhysicsWorld.addStaticBody(glm::vec3(-4.0f, -7.0f, 5.0f));

		for (int j = 0; j < 20; ++j)
		{
			mPhysicsWorld.addStaticBody(glm::vec3(-20.0f + float(j) * 2.0f , -15.0f, 5.0f));
		}

		for (int k = 0; k < 10; ++k)
		{
			mPhysicsWorld.addStaticBody(glm::vec3(-20.0f, -13.0f + float(k) * 2.0f, 5.0f));
		}

		for (int l = 0; l < 10; ++l)
		{
			mPhysicsWorld.addStaticBody(glm::vec3(18.0f, -13.0f + float(l) * 2.0f, 5.0f));
		}

		for (int m = 0; m < 20; ++m)
		{
			mPhysicsWorld.addStaticBody(glm::vec3(-20.0f + float(m) * 2.0f, 7.0f, 5.0f));
		}

		mRenderSystem.registerMeshForBody(RenderSystem::MeshKey::CUBE, mPhysicsWorld.getOccupancy());
		break;

	case Demo::CONTROLLABLE_BOX:
	default: // Defaulted to controllable box demo
		mPhysicsWorld.controllableBoxDemo();
		mRenderSystem.registerMeshForBody(RenderSystem::MeshKey::CUBE, mPhysicsWorld.getOccupancy());
	}

	mDemo = demo;
}

void Application::initUI()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(mpWindowManager->getHandle(), true);
	ImGui_ImplOpenGL3_Init("#version 430");

	// Usually load font here, but let imgui use default front for now.
}

void Application::calculateWorldExtents()
{
	float left, right, top, bottom;
	left   = -6.0f;
	right  =  6.0f;
	top    = -7.8f;
	bottom =  7.8f;

	int width, height;
	glfwGetFramebufferSize(mpWindowManager->getHandle(), &width, &height);

	if (width > height)
	{
		float aspect = static_cast<float>(width) / height;
		left  *= aspect;
		right *= aspect;
	}
	else
	{
		float aspect = static_cast<float>(height) / width;
		top    *= aspect;
		bottom *= aspect;
	}

	mWorldExtentMin = glm::vec2(left, top);
	mWorldExtentMax = glm::vec2(right, bottom);
}

void Application::calculateScale(float *scaleX, float *scaleY, uint32_t frameWidth, uint32_t frameHeight)
{
	*scaleX = (frameWidth  - 1) / (mWorldExtentMax.x - mWorldExtentMin.x);
	*scaleY = (frameHeight - 1) / (mWorldExtentMin.y - mWorldExtentMax.y);
}

void Application::calculateShift(float *shiftX, float *shiftY, uint32_t frameWidth, uint32_t frameHeight)
{
	*shiftX = mWorldExtentMin.x * (1 - static_cast<int>(frameWidth))  / (mWorldExtentMax.x - mWorldExtentMin.x);
	*shiftY = mWorldExtentMin.y * (1 - static_cast<int>(frameHeight)) / (mWorldExtentMax.y - mWorldExtentMin.y);
}

void Application::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		//if (glfwGetInputMode(mpWindowManager->getHandle(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		//{
		//	glfwSetInputMode(mpWindowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		//}
		//else
		//{
		//	glfwSetInputMode(mpWindowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		//}
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if ((key == GLFW_KEY_A    && action == GLFW_PRESS) ||
		(key == GLFW_KEY_LEFT && action == GLFW_PRESS))
	{
		moveLeft = true;
	}
	if ((key == GLFW_KEY_A    && action == GLFW_RELEASE) ||
		(key == GLFW_KEY_LEFT && action == GLFW_RELEASE))
	{
		moveLeft = false;
	}
	if ((key == GLFW_KEY_D     && action == GLFW_PRESS) ||
		(key == GLFW_KEY_RIGHT && action == GLFW_PRESS))
	{
		moveRight = true;
	}
	if ((key == GLFW_KEY_D     && action == GLFW_RELEASE) ||
		(key == GLFW_KEY_RIGHT && action == GLFW_RELEASE))
	{
		moveRight = false;
	}
	if ((key == GLFW_KEY_W  && action == GLFW_PRESS) ||
		(key == GLFW_KEY_UP && action == GLFW_PRESS))
	{
		moveForward = true;
	}
	if ((key == GLFW_KEY_W  && action == GLFW_RELEASE) ||
		(key == GLFW_KEY_UP && action == GLFW_RELEASE))
	{
		moveForward = false;
	}
	if ((key == GLFW_KEY_S    && action == GLFW_PRESS) ||
		(key == GLFW_KEY_DOWN && action == GLFW_PRESS))
	{
		moveBackward = true;
	}
	if ((key == GLFW_KEY_S    && action == GLFW_RELEASE) ||
		(key == GLFW_KEY_DOWN && action == GLFW_RELEASE))
	{
		moveBackward = false;
	}
	if ((key == GLFW_KEY_Q       && action == GLFW_PRESS) ||
		(key == GLFW_KEY_PAGE_UP && action == GLFW_PRESS))
	{
		moveUpward = true;
	}
	if ((key == GLFW_KEY_Q       && action == GLFW_RELEASE) ||
		(key == GLFW_KEY_PAGE_UP && action == GLFW_RELEASE))
	{
		moveUpward = false;
	}

	if ((key == GLFW_KEY_E         && action == GLFW_PRESS) ||
		(key == GLFW_KEY_PAGE_DOWN && action == GLFW_PRESS))
	{
		moveDownward = true;
	}
	if ((key == GLFW_KEY_E         && action == GLFW_RELEASE) ||
		(key == GLFW_KEY_PAGE_DOWN && action == GLFW_RELEASE))
	{
		moveDownward = false;
	}
	// Change camera position and view angle
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		static bool hasChanged = false;

		if (!hasChanged)
		{
			// Set position of the camera to be on the same z as the static box
			mFlyCamera.setPosition(glm::vec3(-50.0f, 2.0f, 5.0f));
			// Look at the static box
			mFlyCamera.setLookAt(glm::vec3(0.0f, -2.0f, 5.0f));
		}
		else
		{
			mFlyCamera.setPosition(glm::vec3(0.0f, 2.0f, 50.0f));
			mFlyCamera.setLookAt(glm::vec3(0.0f, -2.0f, 5.0f));
		}

		mFlyCamera.updateFront();
		mFlyCamera.updateRight();
		mFlyCamera.updateViewMatrix();

		hasChanged = !hasChanged;
	}
}

/**
 * mouseCallback
 * Every time the mouse click, this callback got invoked.
 */
void Application::mouseCallback(GLFWwindow *window, int button, int action, int mods)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) != GLFW_PRESS)
		return;

	if (allowToAdd)
	{
		double posX, posY;
		glfwGetCursorPos(window, &posX, &posY);

		calculateWorldExtents();

		int width, height;
		glfwGetFramebufferSize(mpWindowManager->getHandle(), &width, &height);

		float scaleX, scaleY, shiftX, shiftY;
		scaleX = scaleY = shiftX = shiftY = 0.0f;

		calculateScale(&scaleX, &scaleY, width, height);
		calculateShift(&shiftX, &shiftY, width, height);

		glm::vec2 cursorPosDeviceCoords = glm::vec2(pixelToWorld(posX, scaleX, shiftX), pixelToWorld(posY, scaleY, shiftY));

		mPhysicsWorld.addRigidBody(1, glm::vec3(cursorPosDeviceCoords.x, cursorPosDeviceCoords.y, -15.0f), glm::vec3(0.0f));

		mRenderSystem.registerMeshForBody(RenderSystem::MeshKey::SPHERE, 1u);

		allowToAdd = false;
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
	//if (mIsFirstCursorFocus)
	//{
	//	mLastCursorPosX = xPos;
	//	mLastCursorPosY = yPos;
	//	mIsFirstCursorFocus = false;
	//}
	//mCursorPosDeltaX = static_cast<float>(xPos - mLastCursorPosX);
	//mCursorPosDeltaY = static_cast<float>(mLastCursorPosY - yPos); // Has to be reversed cuz math
	//mFlyCamera.moveView(mCursorPosDeltaX, mCursorPosDeltaY);

	//mLastCursorPosX = xPos;
	//mLastCursorPosY = yPos;
}

void Application::reset()
{
	mModelMatrixContainer.clear();
	mRenderSystem.reset();
	mPhysicsWorld.reset();
	initPhysicsWorld(mDemo);
}

void Application::renderFrame(float dt)
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(mpWindowManager->getHandle(), &width, &height);

	// Prevent assertion when minimize the window
	if (!width && !height) return;

	//static float sideTurn = 0.0f;

	if (moveForward)
	{
		//mFlyCamera.movePosition(Camera::MovementSet::FORWARD, 2.0f * dt);
	}
	if (moveBackward)
	{
		//mFlyCamera.movePosition(Camera::MovementSet::BACKWARD, 2.0f * dt);
	}
	if (moveLeft)
	{
		//if (sideTurn > 0.0f) sideTurn = 0.0f;
		//mFlyCamera.moveView((sideTurn -= 4.0f) * 2.0f * dt, 0.0f);
	}
	if (moveRight)
	{
		//if (sideTurn < 0.0f) sideTurn = 0.0f;
		//mFlyCamera.moveView((sideTurn += 4.0f) * 2.0f * dt, 0.0f);
	}

	mRenderSystem.setView(mFlyCamera.getViewMatrix());

	mRenderSystem.render(mModelMatrixContainer, mpCollisionPairPkg);
	//mRenderSystem.renderInstanced(mModelMatrixContainer);

	if (showHitBoxVerts)
		mRenderSystem.renderDebug(mPhysicsWorld.getBoxColliders(), mpManifoldPkg);
}

void Application::renderUI(double dt)
{
	// Start imgui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Create a window called "Hello, world!" and append into it
	ImGui::Begin("Hello, world!");

	// UI scaling
	ImGui::SetWindowFontScale(1.5f);

	// Update physics tick interval every half a sec.
	static double lastTime = glfwGetTime();
	static double lastPhysicsTickInterval = mPhysicsTickInterval;

	if (glfwGetTime() - lastTime >= 0.5)
	{
		lastPhysicsTickInterval = mPhysicsTickInterval;
		lastTime += 0.5;
	}

	// Display some text
	dt = dt > 0.0001f ? dt : 0.001f; // Prevent divided by 0
	ImGui::Text("FPS: %.3f | Frame time: %.3f ms", 1.0f / dt, dt * 1000.0f);
	ImGui::Text("Physics Tick Rate: %.3f | Physics Tick Interval: %.3f ms",
		1.0f / lastPhysicsTickInterval, lastPhysicsTickInterval * 1000.0f);
	ImGui::Text("Number of objects in world: %d", mPhysicsWorld.getOccupancy());
	ImGui::Text("Number of BoxColliders in world: %d", mPhysicsWorld.getNumBoxColliders());

	if (ImGui::Button("Add"))
		allowToAdd = true;
	if (ImGui::Button("Shoot"))
		shootBall();
	if (ImGui::Button("Reset"))
		reset();
	if (ImGui::Button("Trigger Breakpoint"))
		__debugbreak();
	if (ImGui::Button("Quit"))
		glfwSetWindowShouldClose(mpWindowManager->getHandle(), GL_TRUE);

	ImGui::Checkbox("Show HitBox Verts", &showHitBoxVerts);
	/*ImGui::Checkbox()*/

	// Plot some values
	//const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
	//ImGui::PlotLines("Frame time", my_values, IM_ARRAYSIZE(my_values));

	ImGui::End();

	// Actual rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::shootBall()
{
	mPhysicsWorld.addRigidBody(
		1.0f,
		glm::vec3(mFlyCamera.getPosition() + 1.0f * mFlyCamera.getFront()),
		glm::vec3(0.0f, 0.0f, -10.0f)
	);

	mRenderSystem.registerMeshForBody(RenderSystem::MeshKey::SPHERE, 1u);
}

void Application::update(float dt)
{
	mPhysicsWorld.detectCollisions();

	switch (mDemo)
	{
	case Demo::BOWLING_GAME:
		mPhysicsWorld.updateBowlingGame(dt);
		break;

	case Demo::MULTIPLE_BOXES:
		mPhysicsWorld.updateMultipleBoxes(dt);
		break;

	case Demo::GRAVITY_TEST:
	case Demo::ROTATIONAL_TEST:
		mPhysicsWorld.updateGravityTest(dt);
		break;

	case Demo::CONTROLLABLE_BOX:
	default:
		updateWithInputs(dt);
	}

	mPhysicsTickInterval = dt;

	// Get collision data if needed
	mpCollisionPairPkg = mPhysicsWorld.getPCollisionPairPkg();
	mpManifoldPkg      = mPhysicsWorld.getPManifoldPkg();

	// Update the model matrix array - the bridge from physics quantity to transform matrices for graphics
	// Combine both rigid and static linear transforms
	updateModelMatrices();
}

void Application::updateWithInputs(float dt)
{
	glm::vec3 controlPosition{ 0.0f };

	// Get any inputs for kinematics object
	// Use info from the camera for x-z plane movement
	glm::vec3 cameraRight = mFlyCamera.getRight();
	glm::vec3 cameraFront = mFlyCamera.getFront();

	if (moveForward)
		controlPosition += 5.0f * dt * glm::vec3(cameraFront.x, 0.0f, cameraFront.z);
	if (moveBackward)
		controlPosition -= 5.0f * dt * glm::vec3(cameraFront.x, 0.0f, cameraFront.z);
	if (moveLeft)
		controlPosition -= 5.0f * dt * cameraRight;
	if (moveRight)
		controlPosition += 5.0f * dt * cameraRight;
	if (moveUpward)
		controlPosition.y += 5.0f * dt;
	if (moveDownward)
		controlPosition.y -= 5.0f * dt;

	mPhysicsWorld.updateControllableBox(dt, controlPosition);
}

void Application::updateModelMatrices()
{
	int offset = 0;

	// First wipe, rigid bodies
	for (int i = 0; i < mRigidLinearTransformContainer.size(); ++i)
	{
		glm::mat4 translation = glm::translate(mRigidLinearTransformContainer[i].position);

		if (i >= mModelMatrixContainer.size())
		{
			mModelMatrixContainer.emplace_back(translation);
		}
		else
		{
			mModelMatrixContainer[i] = translation;
		}

		++offset;
	}

	// Angular transforms follow up, indices should sync up with linear transforms
	for (int j = 0; j < mRigidAngularTransformContainer.size(); ++j)
	{
		AngularTransform const &angularTransform = mRigidAngularTransformContainer[j];

		if (angularTransform.angularVelocity == glm::vec3(0.0f))
		{
			mModelMatrixContainer[j] *= glm::mat4(1.0f);
		}
		else
		{
			mModelMatrixContainer[j] *= glm::rotate(angularTransform.tempOrientation, glm::normalize(angularTransform.angularVelocity));
		}
	}

	// Static bodies
	for (int k = 0; k < mStaticLinearTransformContainer.size(); ++k)
	{
		glm::mat4 translation = glm::translate(mStaticLinearTransformContainer[k].position);

		if ((k + offset) >= mModelMatrixContainer.size())
		{
			mModelMatrixContainer.emplace_back(translation);
		}
		else
		{
			mModelMatrixContainer[k + offset] = translation;
		}
	}

	for (int l = 0; l < mStaticLinearTransformContainer.size(); ++l)
	{
		AngularTransform const &angularTransform = mStaticAngularTransformContainer[l];

		if (angularTransform.angularVelocity == glm::vec3(0.0f))
		{
			mModelMatrixContainer[l + offset] *= glm::mat4(1.0f);
		}
		else
		{
			mModelMatrixContainer[l + offset] *= glm::rotate(angularTransform.tempOrientation, glm::normalize(angularTransform.angularVelocity));
		}
	}
}
