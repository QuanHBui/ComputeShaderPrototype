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
#include "Program.h"
#include "Shape.h"
#include "WindowManager.h"

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

	void computeOnGpu();
	void computeOnCpu();

	void render();
	void update(float);

	~Application();

private:
	void initGeom();
	void initCpuBuffers();
	void initGpuBuffers();
	void initRenderProgram();
	void initComputePrograms();

	void updateCpuBuffers(float);
	void updateGpuBuffers();

	void printSsbo();
	void printColorSsbo();
	void interpretComputedSsbo();

	struct Ssbo
	{
		glm::vec4 positionBuffer_A[2763];
		glm::vec4 positionBuffer_B[2763];
		glm::uvec4 elementBuffer_A[5522];
		glm::uvec4 elementBuffer_B[5522];
	} mSsboCpuMem;

	struct Ubo
	{
		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 model_A;
		glm::mat4 model_B;
	} mUboCpuMem;

	struct ColorOutSsbo
	{
		glm::vec4 colorBuffer_A[2763];
		glm::vec4 colorBuffer_B[2763];
	} mColorOutSsbo;


	WindowManager *mpWindowManager = nullptr;
	GLuint mVao, mUboGpuID, mSsboGpuID[2];

	Camera mFlyCamera
	{
		glm::vec3{0.f, 0.f, 0.f},
		glm::vec3{0.f, 0.f, -5.f},
		glm::vec3{0.f, 1.f, 0.f},
		(float)(640 / 480)
	};
	double mLastCursorPosX = 0.0, mLastCursorPosY = 0.0;
	float mCursorPosDeltaX = 0.0f, mCursorPosDeltaY = 0.0f;
	bool mIsFirstCursorFocus = true;

	std::unique_ptr<Program> mpRenderProgram = nullptr;
	std::vector<GLuint> mComputeProgramIDContainer;

	std::vector<std::shared_ptr<Shape>> mMeshContainer;
};

#endif // APPLICATION_H