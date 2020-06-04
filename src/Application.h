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
#include "Program.h"
#include "Shape.h"
#include "WindowManager.h"

void getComputeGroupInfo();

class Application : public EventCallbacks
{
private:
	WindowManager *windowManager = nullptr;
	GLuint	VAO, computeUBOGPU_id, ssboGPU_id[2], renderUBOGPU_id,
			computeProgram_id;

	Camera flyCamera{	glm::vec3{0.f, 0.f, 0.f},
						glm::vec3{0.f, 0.f, -5.f},
						glm::vec3{0.f, 1.f, 0.f},
						(float)(640 / 480)};
	double lastCursorPosX = 0.0, lastCursorPosY = 0.0;
	float cursorPosDeltaX = 0.0f, cursorPosDeltaY = 0.0f;
	bool firstCursorFocus = true;

	std::unique_ptr<Program> renderProgramPtr = nullptr;
	std::vector<std::shared_ptr<Shape>> meshContainer;

	struct SSBO
	{
		glm::vec4 positionBuffer_A[2763];
		glm::vec4 positionBuffer_B[2763];
		glm::uvec4 elementBuffer_A[5522];
		glm::uvec4 elementBuffer_B[5522];
	} ssboCPUMEM;

	struct UBO
	{
		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 model_A;
		glm::mat4 model_B;
	} uboCPUMEM;

	struct ColorOutSSBO
	{
		glm::vec4 colorBuffer_A[2763];
		glm::vec4 colorBuffer_B[2763];
	} colorOutSSBO;

	void printSSBO();
	void printColorSSBO();
	void interpretComputedSSBO();

public:
	~Application();

	void initGeom();
	void initBuffers();
	void initRenderProgram();
	void initComputeProgram();

	void setWindowManager(WindowManager *i_windowManager) { windowManager = i_windowManager; }

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) override;
	void mouseCallback(GLFWwindow *window, int button, int action, int mods) override {};
	void resizeCallback(GLFWwindow *window, int in_width, int in_height) override {};
	void scrollCallback(GLFWwindow *window, double deltaX, double deltaY) override {};
	void cursorCallback(GLFWwindow *window, double xpos, double ypos) override;

	void compute();
	void render();
	void update(float);
};

#endif // APPLICATION_H6