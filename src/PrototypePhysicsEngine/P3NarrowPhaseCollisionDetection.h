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

#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>

#define NUM_NARROW_PHASE_COMPUTE_PROGRAMS 3
#define NUM_NARROW_PHASE_SSBO 5

bool coplanarTriTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &,
						glm::vec3 const &, glm::vec3 const &, glm::vec3 const &,
						glm::vec3 const &);
bool fastTriTriIntersect3DTest(	glm::vec3 const &, glm::vec3 const &, glm::vec3 const &,
								glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
void computeIntersectInterval(	float, float, float,
								float, float, float,
								float, float,
								float &, float &, bool &);
bool edgeEdgeTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
bool edgeTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);
bool pointInTriTest(glm::vec3 const &, glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);


class P3OpenGLComputeNarrowPhase
{
public:
	P3OpenGLComputeNarrowPhase();

	~P3OpenGLComputeNarrowPhase();

private:
	enum { TRI_TRI_TEST = 0 };

	std::array<GLuint, NUM_NARROW_PHASE_COMPUTE_PROGRAMS> computeProgramContainer;
	std::array<GLuint, NUM_NARROW_PHASE_SSBO> ssboContainer;
};

#endif // P3_NARROW_PHASE_COLLISION_DETECTION_H