/**
 * Rigid body simulator
 * @author: Quan Bui
 */
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <iostream>
#include <time.h>

#include "Application.h"
#include "WindowManager.h"

int main()
{
	Application *pApplication = new Application();

	WindowManager *pWindowManager = new WindowManager();
	pWindowManager->init(1080, 810);
	pWindowManager->setEventCallbacks(pApplication);

	pApplication->setWindowManager(pWindowManager);

	try
	{
		pApplication->init();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	double lastTime = glfwGetTime();	// In seconds
	double lastFrameTime = lastTime;
	double dt = 0.0;
	int numFrames = 0;

	// Render and physics loop
	while (!glfwWindowShouldClose(pWindowManager->getHandle()))
	{
		pApplication->update((float)dt);
		pApplication->renderFrame();

		// Measure fps and frame time
		double currentTime = glfwGetTime();
		dt = currentTime - lastFrameTime;
		lastFrameTime = currentTime;
		++numFrames;
		// If last prinf() was more than 3 sec ago
		if (currentTime - lastTime >= 3.0)
		{
			// printf and reset timer
			printf("FPS: %d | Frame time: %f\n", numFrames / 3, 3.0f / (float)numFrames);
			numFrames = 0;
			lastTime += 3.0;
		}

		pApplication->renderUI(dt);

		// Swap front and back buffers.
		glfwSwapBuffers(pWindowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();

		pApplication->update((float)dt);
	}

	// Destroy application before deleting the current OpenGL context
	delete pApplication;
	pApplication = nullptr;

	pWindowManager->shutdown();
	delete pWindowManager;
	pWindowManager = nullptr;

	return 0;
}