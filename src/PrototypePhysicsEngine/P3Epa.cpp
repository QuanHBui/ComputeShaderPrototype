#include "P3Collider.h"

#include "P3Simplex.h"

/**
 * Reference:
 *  http://hacktank.net/blog/?p=119
 *  http://allenchou.net/2013/12/game-physics-contact-generation-epa/
 *
 * @author: Quan Bui
 * @version: 10/18/2020
 */
void P3Epa(P3Collider const &colliderA, P3Collider const &colliderB, P3Simplex &gjkSimplex)
{
	std::vector<SupportPoint> vertices;
	std::vector<TriangleSimplex> triangles;
	std::vector<EdgeSimplex> directedEdges;

	// Blow up simplex to tetrahedron (in the case that the simplex returned
	//  by the gjk doesn't have 4 support points) - however, the current
	//  implementation of gjk guarantees 4 verticies in the case of 3D.
	switch (gjkSimplex.getSize())
	{
	case 1: // No break statement here, we want "case fall-through"
	{
		// How many directions can we look from a single point?
		static const glm::vec3 searchDirections[] =
		{
			glm::vec3{  1.0f,  0.0f,  0.0f },	// Right
			glm::vec3{ -1.0f,  0.0f,  0.0f },	// Left
			glm::vec3{  0.0f,  1.0f,  0.0f },	// Up
			glm::vec3{  0.0f, -1.0f,  0.0f },	// Down
			glm::vec3{  0.0f,  0.0f,  1.0f },	// Forward
			glm::vec3{  0.0f,  0.0f, -1.0f }	// Backward
		};

		// Find a good search direction
		for (glm::vec3 const &searchDirection : searchDirections)
		{
			SupportPoint sup(colliderA, colliderB, searchDirection);

			if (glm::length(sup - gjkSimplex[0]) >= 0.00001f)
			{
				gjkSimplex.pushFront(sup);
				break;
			}
		}
	}

	case 2:
	{
		// How many directions can we look from a single line/edge?
		static const glm::vec3 searchAxes[] =
		{
			glm::vec3{  1.0f,  0.0f,  0.0f },
			glm::vec3{  0.0f,  1.0f,  0.0f },
			glm::vec3{  0.0f,  0.0f,  1.0f }
		};

		// Form the edge
		glm::vec3 edge = gjkSimplex[1] - gjkSimplex[0];

		// Find the least significant axis of above edge

	}

	case 3:
	{
		// How many directions can we look from a single triangle?
	}
	}

	// Fix tetrahedron winding - we want CCW
	// Explanation: https://stackoverflow.com/questions/10612829/tetrahedron-orientation-for-triangle-meshes
	glm::vec3 v30 = gjkSimplex[0] - gjkSimplex[3];
	glm::vec3 v31 = gjkSimplex[1] - gjkSimplex[3];
	glm::vec3 v32 = gjkSimplex[2] - gjkSimplex[3];
	float det = glm::dot(v30, glm::cross(v31, v32));

	if (det > 0.0f)	// Sign of the determinant depends on convention.
		gjkSimplex = { gjkSimplex[1], gjkSimplex[0], gjkSimplex[2], gjkSimplex[3] };

	// Build the initial polytope from the simplex returned from the gjk.
	// Whenever we add a triangle to the list, we add the vertices to
	//  the vertex container. At this point, the winding should be fixed. Fix v30 and v31.
	v30 = gjkSimplex[0] - gjkSimplex[3];
	v31 = gjkSimplex[1] - gjkSimplex[3];

	// Might need to put a hard cap on the number of iterations
	//while (true)
	{
		// Find the facet closest to origin


	}
}