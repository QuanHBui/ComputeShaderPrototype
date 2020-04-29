/**
 * CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
 *
 * This prototype showcases compute shader, specifically:
 *		uniforms in CS
 *		atomic counters
 *		atomic operations
 *		texture handling
 * 		workgroups
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
#include "P3NearPhaseCollisionDetection.h"

#define UNUSED(x) static_cast<void>(x)

float frand()
{
	return (float)rand() / (float)RAND_MAX;
}

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}

int main(int argc, char **argv)
{
	UNUSED(argv);	// Disable command line argument
	srand(static_cast<unsigned int>(time(0)));

	Application *application = new Application();

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1080, 810);
	windowManager->setEventCallbacks(application);

	application->setWindowManager(windowManager);

	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("max global (total) work group size x:%i y:%i z:%i\n\n",
		work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	// application->init();
	// application->initGeom();
	// application->initTex();
	// application->initAtomic();
	// application->compute();
	// application->readAtomic();

	// Unit test triangle-triangle intersection test

	// Definitely collide
	// glm::vec3 v0{-0.5f, 0.0f, 0.0f};
	// glm::vec3 v1{0.0f, 0.5f, 0.0f};
	// glm::vec3 v2{0.5f, 0.0f, 0.0f};

	// glm::vec3 u0{-0.5f, 0.0f, 0.0f};
	// glm::vec3 u1{0.0f, 1.0f, 2.0f};
	// glm::vec3 u2{-0.5f, 0.0f, -2.0f};

	// Definitely not collide
	glm::vec3 v0{-0.5f, 0.0f, 5.0f};
	glm::vec3 v1{0.0f, 0.5f, 5.0f};
	glm::vec3 v2{0.5f, 0.0f, 5.0f};

	glm::vec3 u0{-0.5f, 0.0f, 0.0f};
	glm::vec3 u1{0.0f, 1.0f, 2.0f};
	glm::vec3 u2{-0.5f, 0.0f, -2.0f};

	printf("Collided?\t%s\n\n", fastTriTriIntersect3DTest(v0, v1, v2, u0, u1, u2) ? "true" : "false");

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

	windowManager->shutdown();

	delete application;
	delete windowManager;

	application = nullptr;
	windowManager = nullptr;

	return 0;
}