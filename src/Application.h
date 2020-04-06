/**
 * The graphics application. The highest level of openGL wrapper, a.k.a just another
 *  openGL render engine. This can also be used for general computing purposes (GPGPU),
 *  featuring compute shader.
 *
 * @author: unknown. Assuming professors from Cal Poly Computer Science department
 *  Modified by Quan Bui.
 * @version: 04/05/2020
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

class Application
{
protected:
	WindowManager * windowManager = nullptr;
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

	void resetAtomic();
	void readAtomic();
	void compute();
};

#endif // APPLICATION_H