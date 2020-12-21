/**
 * The graphics application. The highest level of openGL wrapper, a.k.a just another
 *  openGL render engine. This can also be used for general computing purposes (GPGPU),
 *  featuring compute shader.
 *
 * @author: unknown. Assuming professors from Cal Poly Computer Science department.
 *  Compute shader was initially introduced by Christian Eckhardt.
 *  Modified by Quan Bui.
 * @version: 04/22/2020
 */

#pragma once

#ifndef APPLICATION_H
#define APPLICATION_H

#include <glm/glm.hpp>

#include <memory>
#include <vector>

#include "Camera.h"
#include "RenderSystem.h"
#include "PrototypePhysicsEngine/P3DynamicsWorld.h"
#include "WindowManager.h"

#define MAX_NUM_ENTITIES 10u

class Application : public EventCallbacks
{
public:
	void init();

	void setWindowManager(WindowManager *pWindowManager) { mpWindowManager = pWindowManager; }

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) override;
	void mouseCallback(GLFWwindow *window, int button, int action, int mods) override;
	void resizeCallback(GLFWwindow *window, int in_width, int in_height) override {};
	void scrollCallback(GLFWwindow *window, double deltaX, double deltaY) override {};
	void cursorCallback(GLFWwindow *window, double xpos, double ypos) override;

	void reset();

	void renderFrame(float);
	void renderUI(double);

	void shootBall();

	void update(float);

	~Application();

private:
	void initRenderSystem();
	void initPhysicsWorld();
	void initUI();

	// Mouse ultility methods
	void calculateWorldExtents();
	void calculateScale(float *, float *, uint32_t, uint32_t);
	void calculateShift(float *, float *, uint32_t, uint32_t);

	inline float pixelToWorld(double pixel, float scale, float shift)
	{
		return static_cast<float>((pixel - shift) / scale);
	}

	WindowManager *mpWindowManager = nullptr;

	Camera mFlyCamera
	{
		glm::vec3{ 0.0f,  2.0f,  45.0f },
		glm::vec3{ 0.0f, -0.2f,  -1.0f },
		glm::vec3{ 0.0f,  1.0f,   0.0f },
		640.0f / 480.0f
	};

	// Some info states
	double mPhysicsTickInterval = 0.0;

	// World extent
	glm::vec2 mWorldExtentMin = glm::vec2{ 0.0f };
	glm::vec2 mWorldExtentMax = glm::vec2{ 0.0f };

	double mLastCursorPosX = 0.0, mLastCursorPosY = 0.0;
	float mCursorPosDeltaX = 0.0f, mCursorPosDeltaY = 0.0f;
	bool mIsFirstCursorFocus = true;

	RenderSystem mRenderSystem;
	P3DynamicsWorld mPhysicsWorld;

	MatrixContainer mModelMatrixContainer;

	CollisionPairGpuPackage mCollisionPairsFromGpu;
};

#endif // APPLICATION_H
