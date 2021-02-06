#pragma once

#ifndef P3_TRANSFORM_H
#define P3_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/**
 * This aims to be used as a component. Should be an upgrade to the other rigid body class
 * Based on Baraff-Witkins lecture notes and Glenn Fiedler's blog post
 * http://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf
 * https://gafferongames.com/post/physics_in_3d/
 *
 * @author: Quan Bui
 * @version: 09/28/2020
 */

struct LinearTransform
{
	LinearTransform() {};
	LinearTransform(float, float, glm::vec3 const &, glm::vec3 const &, glm::vec3 const &);

	//----------------- Constant quantities -----------------//
	float mass = 0.0f;
	float inverseMass = 0.0f;

	//----------------- State variables -----------------//
	glm::vec3 position{};
	glm::vec3 velocity{};
	glm::vec3 momentum{};
};

//------------------ Data pack for the GPU (SoA) --------------------//
struct LinearTransformGpuPackage
{
	glm::vec4 positions[cMaxObjectCount]{};
	glm::vec4 velocities[cMaxObjectCount]{};
	glm::vec4 masses[cMaxObjectCount]{};
};

struct AngularTransform
{
	//----------------- Constant quantities -----------------//
	float inertia = 0.0f;
	float inverseInertia = 0.0f;

	//----------------- State variables -----------------//
	float tempOrientation = 0.0f; // Placeholder for rotational angle
	glm::quat orientation{};
	glm::quat spin{};
	glm::vec3 angularVelocity{};
	glm::vec3 angularMomentum{};
};

#endif // P3_TRANSFORM_H