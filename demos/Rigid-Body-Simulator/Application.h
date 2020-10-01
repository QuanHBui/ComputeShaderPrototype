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

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

#include "Camera.h"
#include "Systems/RenderSystem.h"
#include "PrototypePhysicsEngine/P3DynamicsWorld.h"
#include "WindowManager.h"

#define MAX_NUM_ENTITIES 10u

class Application : public EventCallbacks
{
public:
	void init();

	void setWindowManager(WindowManager *pWindowManager) { mpWindowManager = pWindowManager; }

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) override;
	void mouseCallback(GLFWwindow *window, int button, int action, int mods) override {};
	void resizeCallback(GLFWwindow *window, int in_width, int in_height) override {};
	void scrollCallback(GLFWwindow *window, double deltaX, double deltaY) override {};
	void cursorCallback(GLFWwindow *window, double xpos, double ypos) override;

	void renderFrame();
	void renderUI(double);
	void update(float);

	~Application();

private:
	void initRenderSystem();
	void initUI();

	WindowManager *mpWindowManager = nullptr;

	Camera mFlyCamera
	{
		glm::vec3{ 0.f, 1.f, 5.f },
		glm::vec3{ 0.f, 0.f, -5.f },
		glm::vec3{ 0.f, 1.f, 0.f },
		(float)(640 / 480)
	};
	double mLastCursorPosX = 0.0, mLastCursorPosY = 0.0;
	float mCursorPosDeltaX = 0.0f, mCursorPosDeltaY = 0.0f;
	bool mIsFirstCursorFocus = true;

	RenderSystem renderSystem;
	P3DynamicsWorld physicsWorld;

	std::shared_ptr<MatrixContainer> pModelMatrixContainer = nullptr;
};

#endif // APPLICATION_H