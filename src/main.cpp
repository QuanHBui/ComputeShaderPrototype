/**
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

void getComputeGroupInfo()
{
	int work_grp_cnt[3];

	CHECKED_GL_CALL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]));
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("max global (total) work group counts x:%i y:%i z:%i\n",
		work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	int work_grp_size[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	printf("max local (in one shader) work group size x:%i y:%i z:%i \n",
		work_grp_size[0], work_grp_size[1], work_grp_size[2]);

	int work_grp_inv;

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	printf("max local work group invocations %i\n", work_grp_inv);

	int max_shader_storage_buffer_bindings;

	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &max_shader_storage_buffer_bindings);
	printf("max shader storage buffer bindings %i\n", max_shader_storage_buffer_bindings);
}

int main(int argc, char **argv)
{
	UNUSED(argv);	// Disable command line argument

	Application *application = new Application();

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1080, 810);
	windowManager->setEventCallbacks(application);

	application->setWindowManager(windowManager);

	application->init();
	// application->initGeom();
	application->initSSBO();
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