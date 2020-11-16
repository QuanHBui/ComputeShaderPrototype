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

#include "ComputeProgram.h"

constexpr uint16_t num_narrow_phase_compute_programs = 1u;
constexpr GLsizei  num_narrow_phase_ssbos            = 1u;

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
		mBoxCollidersID   = boxCollidersID;
		mCollisionPairsID = collisionPairsID;

		initShaderPrograms();
		initGpuBuffers();
	}

	void step(uint16_t);
	void reset();

	~P3OpenGLComputeNarrowPhase()
	{
		for (uint16_t i = 0u; i < num_narrow_phase_compute_programs; ++i)
			glDeleteProgram(mComputeProgramIDs[i]);

		glDeleteBuffers(num_narrow_phase_ssbos, mSsboIDs);
	}

private:
	void initShaderPrograms()
	{
		mComputeProgramIDs[SAT] = createComputeProgram("../resources/shaders/SAT.comp");
	}

	void initGpuBuffers();

	enum
	{
		SAT = 0,
		TRI_TRI_TEST
	};

	enum
	{
		MTVS = 0
	};

	// Some Gpu buffers already created from the broad phase
	GLuint mBoxCollidersID = 0u, mCollisionPairsID = 0u;

	GLuint mComputeProgramIDs[num_narrow_phase_compute_programs];
	GLuint mSsboIDs[num_narrow_phase_ssbos];
};

#endif // P3_NARROW_PHASE_COLLISION_DETECTION_H
