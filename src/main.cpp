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

#define GPU_COMPUTE true

int main(int argc, char **argv)
{
	UNUSED(argv);	// Disable command line argument

	Application *application = new Application();

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1080, 810);
	windowManager->setEventCallbacks(application);

	application->setWindowManager(windowManager);

	application->init();

	double lastTime = glfwGetTime();
	double lastFrameTime = lastTime;
	double dt = 0.0;
	int numFrames = 0;

	// Render and computeloop
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		if (GPU_COMPUTE) application->computeOnGpu();
		else application->computeOnCpu();

		application->render();

		// Measure fps and frame time
		double currentTime = glfwGetTime();
		dt = currentTime - lastFrameTime;
		lastFrameTime = currentTime;
		++numFrames;
		// If last prinf() was more than 3 sec ago
		if (currentTime - lastTime >= 3.0)
		{
			// printf and reset timer
			printf("FPS: %d | Frame time: %f\n", numFrames/3, 3.0f/(float)numFrames);
			numFrames = 0;
			lastTime += 3.0;
		}

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();

		application->update((float)dt);
	}

	// Destroy application before deleting the current OpenGL context
	delete application;
	application = nullptr;

	windowManager->shutdown();
	delete windowManager;
	windowManager = nullptr;

	return 0;
}