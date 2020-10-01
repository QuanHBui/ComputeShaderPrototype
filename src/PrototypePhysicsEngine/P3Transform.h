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
	//----------------- Constant quantities -----------------//
	float mass, inverseMass;

	//----------------- State variables -----------------//
	glm::vec3 position, velocity, momentum;

	inline void recalculate()
	{
		velocity = momentum * inverseMass;
	}
};

struct LinearTransformDerivate
{
	glm::vec3 velocity, force;
};

struct AngularTransform
{
	//----------------- Constant quantities -----------------//
	float inertia, inverseInertia;

	//----------------- State variables -----------------//
	glm::quat orientation, spin;
	glm::vec3 angularVelocity, angularMomentum;

	inline void recalculate()
	{
		angularVelocity = angularMomentum * inverseInertia;
		
		/*orientation.normalize();*/
		glm::quat q(0, angularVelocity.x, angularVelocity.y, angularVelocity.z);

		spin = 0.5f * q * orientation;
	}
};

struct AngularTransformDerivative
{
	glm::quat spin;
	glm::vec3 torque;
};

inline glm::vec3 calculateTorque(AngularTransform &state, double t)
{
	return glm::vec3(1.0f, 0.0f, 0.0f) - state.angularVelocity * 0.1f;
}

#endif // P3_TRANSFORM_H