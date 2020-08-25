/**
 * Unit test for triangle triangle intersection test
 *
 * @author: Quan Bui
 * @version: 05/4/2020
 */

#include <cstdio>
#include <glm/vec3.hpp>

#include "P3NarrowPhaseCollisionDetection.h"

void fastTriTriUnitTestCollide()
{
	// Definitely collide
	glm::vec3 v0{ -0.5f, 0.0f, 0.0f };
	glm::vec3 v1{ 0.0f, 0.5f, 0.0f };
	glm::vec3 v2{ 0.5f, 0.0f, 0.0f };

	glm::vec3 u0{ -0.5f, 0.0f, 0.0f };
	glm::vec3 u1{ 0.0f, 1.0f, 2.0f };
	glm::vec3 u2{ -0.5f, 0.0f, -2.0f };

	printf("Collided?\t%s\n\n", fastTriTriIntersect3DTest(v0, v1, v2, u0, u1, u2) ? "true" : "false");
}

void fastTriTriUnitTestNotCollide()
{
	// Definitely not collide
	glm::vec3 v0{ -0.5f, 0.0f, 5.0f };
	glm::vec3 v1{ 0.0f, 0.5f, 5.0f };
	glm::vec3 v2{ 0.5f, 0.0f, 5.0f };

	glm::vec3 u0{ -0.5f, 0.0f, 0.0f };
	glm::vec3 u1{ 0.0f, 1.0f, 2.0f };
	glm::vec3 u2{ -0.5f, 0.0f, -2.0f };

	printf("Collided?\t%s\n\n", fastTriTriIntersect3DTest(v0, v1, v2, u0, u1, u2) ? "true" : "false");
}

inline void fastTriTriUnitTest(	glm::vec3 const &v0, glm::vec3 const &v1, glm::vec3 const &v2,
								glm::vec3 const &u0, glm::vec3 const &u1, glm::vec3 const &u2)
{
	printf("Collided?\t%s\n\n", fastTriTriIntersect3DTest(v0, v1, v2, u0, u1, u2) ? "true" : "false");
}