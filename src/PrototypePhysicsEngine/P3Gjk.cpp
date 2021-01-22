#include "P3Collider.h"

#include <glm/glm.hpp>

#include "P3Simplex.h"

#define SAME_DIRECTION(a, b) glm::dot(a, b) > 0.0001f

bool checkLine(P3Simplex &gjkSimplex, glm::vec3 &direction)
{
	SupportPoint supA = gjkSimplex[0];
	SupportPoint supB = gjkSimplex[1];

	glm::vec3 ab = supB - supA;
	glm::vec3 ao = -supA.mMinkowskiDiffPoint; // From 'a' to origin

	// If origin is in ao, we keep support point b and a, then
	//  we choose another support point in the direction that
	//  spans another dimension, a.k.a a triangle in 2D.
	if (SAME_DIRECTION(ab, ao))
	{
		direction = glm::cross(ao, ab);
	}
	// If not we replace b with another support point in direction of ao
	//  and start again.
	else
	{
		gjkSimplex = { supA };
		direction = ao;
	}

	return false;
}

bool checkTriangle(P3Simplex &gjkSimplex, glm::vec3 &direction)
{
	SupportPoint supA = gjkSimplex[0];
	SupportPoint supB = gjkSimplex[1];
	SupportPoint supC = gjkSimplex[2];

	glm::vec3 ab = supB - supA;
	glm::vec3 ac = supC - supA;
	glm::vec3 ao = -supA.mMinkowskiDiffPoint;;

	glm::vec3 abc = glm::cross(ab, ac);

	// TODO: Need to understand what's going on
	if (SAME_DIRECTION(glm::cross(abc, ac), ao)) {
		if (SAME_DIRECTION(ac, ao)) {
			gjkSimplex = { supA, supC };
			direction = glm::cross(glm::cross(ac, ao), ac);
		}
		else
		{
			return checkLine(gjkSimplex = { supA, supB }, direction);
		}
	}
	else
	{
		if (SAME_DIRECTION(glm::cross(ab, abc), ao))
		{
			return checkLine(gjkSimplex = { supA, supB }, direction);
		}
		else
		{
			if (SAME_DIRECTION(abc, ao))
			{
				direction = abc;
			}
			else
			{
				gjkSimplex = { supA, supC, supB };
				direction = -abc;
			}
		}
	}

	return false;
}

// TODO: Need to understand what's going on
bool checkTetrahedron(P3Simplex &gjkSimplex, glm::vec3 &direction)
{
	SupportPoint supA = gjkSimplex[0];
	SupportPoint supB = gjkSimplex[1];
	SupportPoint supC = gjkSimplex[2];
	SupportPoint supD = gjkSimplex[3];

	glm::vec3 ab = supB - supA;
	glm::vec3 ac = supC - supA;
	glm::vec3 ad = supD - supA;
	glm::vec3 ao = -supA.mMinkowskiDiffPoint;

	glm::vec3 abc = glm::cross(ab, ac);
	glm::vec3 acd = glm::cross(ac, ad);
	glm::vec3 adb = glm::cross(ad, ab);

	if (SAME_DIRECTION(abc, ao))
		return checkTriangle(gjkSimplex = { supA, supB, supC }, direction);

	if (SAME_DIRECTION(acd, ao))
		return checkTriangle(gjkSimplex = { supA, supC, supD }, direction);

	if (SAME_DIRECTION(adb, ao))
		return checkTriangle(gjkSimplex = { supA, supD, supB }, direction);

	return true;
}

bool nextSimplex(P3Simplex &gjkSimplex, glm::vec3 &direction)
{
	switch (gjkSimplex.getSize())
	{
	case 2: return checkLine(gjkSimplex, direction);
	case 3: return checkTriangle(gjkSimplex, direction);
	case 4: return checkTetrahedron(gjkSimplex, direction);
	}

	return false;
}

/**
 * Highest level of GJK algorithm
 *
 * Reference: https://blog.winter.dev/2020/gjk-algorithm/
 */
bool P3Gjk(P3Collider const &colliderA, P3Collider const &colliderB, P3Simplex &gjkSimplex)
{
	// Get inital support point in an arbitrary direction
	SupportPoint supportPoint(colliderA, colliderB, glm::vec3(1.0f, 0.0f, 0.0f));

	// Add support point to the simplex and get the new search direction
	gjkSimplex.pushFront(supportPoint);
	glm::vec3 direction = -supportPoint.mMinkowskiDiffPoint;

	// Find new support point. If this new support point not in front of the search direction,
	//  the loop ends.
	while (true)
	{
		supportPoint = SupportPoint(colliderA, colliderB, direction);

		// TODO: Wait what would happen if the direction vector is zero, i.e. in the case where
		//  the first support point happens to be the origin??
		// Origin not included in the Minkowski difference, so no collision.
		if (glm::length(direction) > 0.0001f && glm::dot(supportPoint.mMinkowskiDiffPoint, direction) <= 0.0001f)
			return false;

		gjkSimplex.pushFront(supportPoint);

		// Now we have simplex (line -> triangle -> tetrahedral), feed into a function to update
		//  the simplex and search direction.
		if (nextSimplex(gjkSimplex, direction))
			return true;
	}
}