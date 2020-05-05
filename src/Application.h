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

#include "WindowManager.h"

#include <glm/glm.hpp>

#ifndef STAR_COUNT
#define STAR_COUNT 1024
#endif

struct SSBO
{
	glm::vec4 vertexBuffer_A[STAR_COUNT];
	glm::vec4 vertexBuffer_B[STAR_COUNT];
	glm::uvec4 elementBuffer_A[STAR_COUNT];
	glm::uvec4 elementBuffer_B[STAR_COUNT];
};

class Application : public EventCallbacks
{
private:
	WindowManager *windowManager = nullptr;
	GLuint ssboGPU_id;
	GLuint computeProgram_id;
	SSBO ssboCPUMEM;

public:
	~Application();

	void init();
	void initGeom();
	void initSSBO();

	void setWindowManager(WindowManager *i_windowManager) { windowManager = i_windowManager; }

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) override;
	void mouseCallback(GLFWwindow *window, int button, int action, int mods) override {};
	void resizeCallback(GLFWwindow *window, int in_width, int in_height) override {};
	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) override {};
	void cursorCallback(GLFWwindow* window, double xpos, double ypos) override {};

	void compute();
	void render();
};

#endif // APPLICATION_H