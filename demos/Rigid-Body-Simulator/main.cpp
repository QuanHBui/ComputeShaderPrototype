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
	Application* pApplication = new Application();

	WindowManager* pWindowManager = new WindowManager();
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
		glfwSetWindowShouldClose(pWindowManager->getHandle(), GL_TRUE);
	}

	double lastTime = glfwGetTime();	// In seconds
	double lastFrameTime = lastTime;
	double frameTimeInterval = 0.0;
	double UIFrameTimeInterval = frameTimeInterval;
	int numFrames = 0;

	double remainingFrameTimeInterval = 0.0;
	double fixedPhysicsTickInterval = 0.0167;	// 1/60.0 sec
	// Semi-fix timestep for physics simulation
	// Render and physics loop
	while (!glfwWindowShouldClose(pWindowManager->getHandle()))
	{
		pApplication->update((float)frameTimeInterval);
		pApplication->renderFrame();

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

		remainingFrameTimeInterval = frameTimeInterval;
		while (remainingFrameTimeInterval > 0.0) 
		{
			double actualPhysicsTickInterval = 
				frameTimeInterval < fixedPhysicsTickInterval ? frameTimeInterval : fixedPhysicsTickInterval;
			pApplication->update(float(actualPhysicsTickInterval));
			remainingFrameTimeInterval -= fixedPhysicsTickInterval;
		}
	}

	// Destroy application before deleting the current OpenGL context
	delete pApplication;
	pApplication = nullptr;

	pWindowManager->shutdown();
	delete pWindowManager;
	pWindowManager = nullptr;

	return 0;
}