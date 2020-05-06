﻿/**
 * CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
 *
 * This prototype showcases compute shader, specifically:
 *		uniforms in CS
 *		atomic counters
 *		atomic operations
 *		texture handling
 *		workgroups
 *
 * Modified by Quan Bui
 */

#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <iostream>
#include <time.h>
#include <glm/glm.hpp>

#include "Program.h"	// This shouldn't be here
#include "Application.h"
#include "GLSL.h"
#include "P3NarrowPhaseCollisionDetection.h"

#define UNUSED(x) static_cast<void>(x)

int main(int argc, char **argv)
{
	UNUSED(argv);	// Disable command line argument

	Application *application = new Application();

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1080, 810);
	windowManager->setEventCallbacks(application);

	application->setWindowManager(windowManager);

	application->initGeom();
	application->initSSBO();
	application->initRenderProgram();
	application->initComputeProgram();
	application->compute();

	// Render loop
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Destroy application before deleting the current OpenGL context
	delete application;
	application = nullptr;

	windowManager->shutdown();
	delete windowManager;
	windowManager = nullptr;

	return 0;
}