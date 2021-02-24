/**
 * Rigid body simulator
 * @author: Quan Bui
 */
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define _USE_MATH_DEFINES

#include <iostream>
#include <time.h>

#include <glad/glad.h>

#include "Application.h"
#include "WindowManager.h"

// 2 query buffers: front and back
#define QUERY_BUFFER_COUNT 2
GLuint queryIDs[QUERY_BUFFER_COUNT] = { 0, 0 };
GLuint queryBackBuffer = 0, queryFrontBuffer = 1;

// Call this as a part of initializing the application, must be after a context has been created.
void genQueries()
{
	glGenQueries(2, queryIDs);

	glBeginQuery(GL_TIME_ELAPSED, queryIDs[queryFrontBuffer]);
	glEndQuery(GL_TIME_ELAPSED);

	glBeginQuery(GL_TIME_ELAPSED, queryIDs[queryBackBuffer]);
	glEndQuery(GL_TIME_ELAPSED);
}

void swapQueryBuffers()
{
	if (queryBackBuffer)
	{
		queryBackBuffer  = 0;
		queryFrontBuffer = 1;
	}
	else
	{
		queryBackBuffer  = 1;
		queryFrontBuffer = 0;
	}
}

int main()
{
	srand(time(NULL));

	Application *pApplication = new Application();

	WindowManager *pWindowManager = new WindowManager();
	pWindowManager->init(1920, 1080);
	pWindowManager->setEventCallbacks(pApplication);

	GLFWwindow *pCurrentGlfwWindow = pWindowManager->getHandle();

	pApplication->setWindowManager(pWindowManager);

	pApplication->init();

	double lastTime            = glfwGetTime(); // In seconds
	double lastFrameTime       = lastTime;
	double frameTimeInterval   = 0.0;
	double UIFrameTimeInterval = frameTimeInterval;
	int    numFrames           = 0;

	// Semi-fix timestep for physics simulation
	double lastPhysicsTickTime      = lastTime;
	double fixedPhysicsTickInterval = 1.0 / 144.0;
	int physicsTickCount = 0;

	GLuint64 gpuTime = 0;
	genQueries();

	// Render and physics loop
	while (!glfwWindowShouldClose(pCurrentGlfwWindow))
	{
		// Measure fps and frame time
		double currentTime = glfwGetTime();
		frameTimeInterval  = currentTime - lastFrameTime;

		lastFrameTime = currentTime;
		++numFrames;

		// Start the query and tell OpenGL to use the back buffer to store the result
		glBeginQuery(GL_TIME_ELAPSED, queryIDs[queryBackBuffer]);

		double realPhysicsTickInterval = currentTime - lastPhysicsTickTime;
		if (realPhysicsTickInterval >= fixedPhysicsTickInterval || !physicsTickCount)
		{
			pApplication->update(float(realPhysicsTickInterval));
			lastPhysicsTickTime = currentTime;
			++physicsTickCount;
		}

		pApplication->renderFrame(float(frameTimeInterval));

		// End time query and store the result
		glEndQuery(GL_TIME_ELAPSED);

		// Show updated FPS/Frame time on UI every half a sec.
		if (currentTime - lastTime >= 0.5)
		{
			UIFrameTimeInterval = 0.5 / numFrames;
			numFrames = 0;
			lastTime += 0.5;

			// Get result from the front buffer
			glGetQueryObjectui64v(queryIDs[queryFrontBuffer], GL_QUERY_RESULT, &gpuTime);
		}

		pApplication->renderUI(UIFrameTimeInterval, gpuTime);

		swapQueryBuffers();

		// Swap front and back buffers.
		glfwSwapBuffers(pCurrentGlfwWindow);
		// Poll for and process events.
		glfwPollEvents();
	}

	// Destroy application before deleting the current OpenGL context
	delete pApplication;
	pApplication = nullptr;

	pWindowManager->shutdown();
	delete pWindowManager;
	pWindowManager = nullptr;

	return 0;
}
