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

class ssbo_data
{
public:
	glm::vec4 dataA[STAR_COUNT];
	glm::ivec4 dataB[STAR_COUNT];
};

class Application : public EventCallbacks
{
protected:
	WindowManager *windowManager = nullptr;
	GLuint texture;
	ssbo_data ssbo_CPUMEM;
	GLuint ssbo_GPU_id;
	GLuint computeProgram;
	GLuint atomicsBuffer;

public:
	void init();
	void initGeom();
	void initAtomic();
	void initTex();

	void setWindowManager(WindowManager *i_windowManager) { windowManager = i_windowManager; }
	void resetAtomic();
	void readAtomic();
	void compute();

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) override;
	void mouseCallback(GLFWwindow *window, int button, int action, int mods) override {};
	void resizeCallback(GLFWwindow *window, int in_width, int in_height) override {};
	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) override {};
	void cursorCallback(GLFWwindow* window, double xpos, double ypos) override {};

	void render();
};

#endif // APPLICATION_H