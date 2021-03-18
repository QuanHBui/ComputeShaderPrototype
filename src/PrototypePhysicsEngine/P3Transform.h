#pragma once

#ifndef P3_TRANSFORM_H
#define P3_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "P3Common.h"

// @source: Game Physics Engine Development by Ian Millington
inline void addScaledVector(glm::quat &ogQuat, glm::vec3 const &v, float scale)
{
	glm::quat q(0, v.x * scale, v.y * scale, v.z * scale);

	ogQuat += q * 0.5f;
}

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
	//----------------- Constant quantities -----------------//
	float mass = 0.0f;
	float inverseMass = 0.0f;

	// To be ignored
	float paddings[2] = { 0.0f, 0.0f };

	//----------------- State variables -----------------//
	glm::vec4 position{};
	glm::vec4 velocity{};
	glm::vec4 momentum{};
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
	glm::mat4 inertia{1.0f};
	glm::mat4 inverseInertia{1.0f};

	//----------------- State variables -----------------//
	glm::quat orientation{};
	glm::vec4 angularVelocity{};
	glm::vec4 angularMomentum{};
};

#endif // P3_TRANSFORM_H
