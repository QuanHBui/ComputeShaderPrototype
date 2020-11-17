/**
 * 3D Triangle-triangle intersection test
 *  There are 2 special cases to worry about: (1) Degenerate tri input, (2) coplanar tri-tri
 *
 * @author: Quan Bui
 * @version: 04/28/2020
 * @reference: 	Tomas Moller, "A Fast Triangle-Triangle Intersection Test"
 *				https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/pubs/tritri.pdf
 */

#pragma once

#ifndef P3_NARROW_PHASE_COLLISION_DETECTION_H
#define P3_NARROW_PHASE_COLLISION_DETECTION_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "AtomicCounter.h"
#include "ComputeProgram.h"

constexpr uint16_t narrow_phase_compute_program_count = 1u;
constexpr GLsizei  narrow_phase_ssbo_count            = 1u;

bool coplanarTriTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &,
						glm::vec3 const &, glm::vec3 const &, glm::vec3 const &,
						glm::vec3 const &);
bool fastTriTriIntersect3DTest(	glm::vec3 const &, glm::vec3 const &, glm::vec3 const &,
								glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
void computeIntersectInterval(float, float, float,
							  float, float, float,
							  float, float,
							  float &, float &, bool &);
bool edgeEdgeTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
bool edgeTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
bool pointInTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);


struct BoundingVolume;

struct ContactManifold
{

};


void buildContactManifold(ContactManifold &, BoundingVolume *, BoundingVolume *);


class P3OpenGLComputeNarrowPhase
{
public:
	void init(GLuint boxCollidersID, GLuint collisionPairsID)
	{
		assert(boxCollidersID && collisionPairsID && "Invalid input buffer handles.");

		mBoxCollidersID   = boxCollidersID;
		mCollisionPairsID = collisionPairsID;

		initShaderPrograms();
		initGpuBuffers();

		atomicCounter.init();
	}

	void step(uint16_t);
	void reset();

	~P3OpenGLComputeNarrowPhase()
	{
		for (uint16_t i = 0u; i < narrow_phase_compute_program_count; ++i)
			glDeleteProgram(mComputeProgramIDs[i]);

		glDeleteBuffers(narrow_phase_ssbo_count, mSsboIDs);
	}

private:
	void initShaderPrograms()
	{
		mComputeProgramIDs[SAT] = createComputeProgram("../resources/shaders/SAT.comp");
	}

	void initGpuBuffers();

	enum ComputeShader: uint8_t
	{
		SAT = 0,
		TRI_TRI_TEST
	};

	enum StorageShaderBuffer: uint8_t
	{
		MANIFOLDS = 0,
		MTVS
	};

	// Some Gpu buffers already created from the broad phase
	GLuint mBoxCollidersID = 0u, mCollisionPairsID = 0u;

	GLuint mComputeProgramIDs[narrow_phase_compute_program_count];
	GLuint mSsboIDs[narrow_phase_ssbo_count];

	AtomicCounter atomicCounter;
};

#endif // P3_NARROW_PHASE_COLLISION_DETECTION_H
