/**
 * CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
 *
 * This prototype showcases compute shader, specifically:
 *		UBO
 *		SSBO
 *		workgroups
 *
 * Modified by Quan Bui
 */

#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <time.h>

#include "Application.h"
#include "GLSL.h"

#define UNUSED(x) static_cast<void>(x)

int main(int argc, char **argv)
{
	UNUSED(argv);	// Disable command line argument

	Application *application = new Application();

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1080, 810);
	windowManager->setEventCallbacks(application);

	application->setWindowManager(windowManager);

	// Must be called strictly in this order
	application->initGeom();
	application->initSSBO();
	application->initComputeProgram();
	application->initRenderProgram();

	// application->compute();

	double lastTime = glfwGetTime();
 	int numFrames = 0;

	// Render and computeloop
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		application->compute();
		application->render();

		// Measure fps and frame time
		double currentTime = glfwGetTime();
		++numFrames;
		// If last prinf() was more than 3 sec ago
		if (currentTime - lastTime >= 3.0) {
			// printf and reset timer
			printf("FPS: %d | Frame time: %f\n", numFrames/3, 3.0f/(float)numFrames);
			numFrames = 0;
			lastTime += 3.0;
		}

		application->update();

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