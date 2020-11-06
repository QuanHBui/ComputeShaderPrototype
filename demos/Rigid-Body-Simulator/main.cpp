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
	srand(time(NULL));

	Application *pApplication = new Application();

	WindowManager *pWindowManager = new WindowManager();
	pWindowManager->init(1920, 1080);
	pWindowManager->setEventCallbacks(pApplication);

	pApplication->setWindowManager(pWindowManager);

	pApplication->init();

	double lastTime = glfwGetTime();	// In seconds
	double lastFrameTime = lastTime;
	double frameTimeInterval = 0.0;
	double UIFrameTimeInterval = frameTimeInterval;
	int numFrames = 0;

	double lastPhysicsTickTime = lastTime;
	double fixedPhysicsTickInterval = 1.0 / 40.0;
	// Semi-fix timestep for physics simulation
	// Render and physics loop
	while (!glfwWindowShouldClose(pWindowManager->getHandle()))
	{
		// Measure fps and frame time
		double currentTime = glfwGetTime();
		frameTimeInterval = currentTime - lastFrameTime;

		lastFrameTime = currentTime;
		++numFrames;

		// Show updated FPS/Frame time on UI ever half a sec.
		if (currentTime - lastTime >= 0.5)
		{
			UIFrameTimeInterval = 0.5 / numFrames;
			numFrames = 0;
			lastTime += 0.5;
		}

		pApplication->renderUI(UIFrameTimeInterval);

		// Swap front and back buffers.
		glfwSwapBuffers(pWindowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
		if (currentTime - lastPhysicsTickTime >= fixedPhysicsTickInterval)
		{
			pApplication->update(float(fixedPhysicsTickInterval));
			lastPhysicsTickTime += fixedPhysicsTickInterval;
		}

		pApplication->renderFrame(float(frameTimeInterval));
	}

	// Destroy application before deleting the current OpenGL context
	delete pApplication;
	pApplication = nullptr;

	pWindowManager->shutdown();
	delete pWindowManager;
	pWindowManager = nullptr;

	return 0;
}
