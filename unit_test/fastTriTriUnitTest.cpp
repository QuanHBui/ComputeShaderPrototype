/**
 * Unit test for triangle triangle intersection test
 *
 * @author: Quan Bui
 * @version: 05/4/2020
 */

#include <glm/glm.hpp>

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