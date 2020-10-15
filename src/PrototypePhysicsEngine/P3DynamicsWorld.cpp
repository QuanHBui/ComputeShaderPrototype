/**
 * Implementation of the DynamicsWorld
 * For now just check collision between 2 Dynamic bodies
 * @author: Quan Bui
 * @version: 02/26/2020
 */

#include "P3DynamicsWorld.h"

#include <iostream>

#include "P3Gjk.h"

 // Order of operations for each timestep: Collision -> apply forces -> solve constraints -> update positions
void P3DynamicsWorld::step(double dt)
{
	// To define a plane, we need a normal and a point
	glm::vec3 surfaceNormal{ 0.0f, 1.0f, 0.0f };
	glm::vec3 surfacePoint{ 0.0f, 1.0f, 0.0f };

	// There will be another outer loop to iterate through how many iterations to reach convergence.
	// For 1 body and 1 position constraint.
	for (RigidBody const& rigidBody : mBodyContainer)
	{
		LinearTransform& linearTransform = mLinearTransformContainer[rigidBody];
		
		// Collision detection - broad phase + near phase
		bool hasCollided = false;
		if (rigidBody < 5u && getOccupancy() > 5u)
		{
			for (unsigned int i = 5u; i < getOccupancy(); ++i)
			{
				hasCollided = gjk(&mMeshColliderContainer[rigidBody], &mMeshColliderContainer[i]);
			}
		}

		glm::vec3 accumulateImpulse{ 0.0f };

		// Apply forces - gravity most likely
		if (hasCollided)
			accumulateImpulse.y += 0.10f;

		linearTransform.velocity.y -= 9.81f * dt;
		linearTransform.momentum.y = linearTransform.mass * linearTransform.velocity.y;

		// Check for constraint and solve it
		// MULTIPLE ITERATIONS
		// NO BOUNCE!
		int iterations = 4;
		glm::vec3 sampleVelocity = linearTransform.velocity;
		glm::vec3 samplePosition = linearTransform.position;
		while (iterations--)
		{
			// Update position of rigid body after apply forces
			samplePosition += sampleVelocity * float(dt);

			// Express the constraint in term of position. This is a position constraint.
			if (samplePosition.y - (-3.0f) < 0.0f)
			{
				// We have to modify the accumulate velocity to solve this position constraint
				// How much in the y direction that we have to push the object, this y direction can be generalize
				//  to just the surface/plane normal.
				// THE IMPULSE CAN PUSH, BUT NOT PULL.
				float signedDistance = samplePosition.y - (-3.0f);
				//accumulateImpulse += glm::vec3(0.0f, -signedDistance, 0.0f);

				// Has the constraint been satisfied
				//sampleVelocity += accumulateImpulse * linearTransform.inverseMass;
				//samplePosition += sampleVelocity * float(dt);

				samplePosition.y -= signedDistance;
				linearTransform.position = samplePosition;
				// Make current velocity zero
				accumulateImpulse += glm::length(sampleVelocity) * glm::normalize(glm::vec3(0.0f, -signedDistance, 0.0f));
			}
		}

		// Apply the final impulse
		linearTransform.velocity += accumulateImpulse * linearTransform.inverseMass;

		// Integrate velocity to get change in position
		linearTransform.position += linearTransform.velocity * float(dt);
		mMeshColliderContainer[rigidBody].setModelMatrix(glm::translate(glm::mat4(1.0f), linearTransform.position));
	}

	//std::cout << mLinearTransformContainer[0].position.y << std::endl;
}

void P3DynamicsWorld::addRigidBody()
{
	// Generate unique ID and add to ID container
	mBodyContainer.emplace_back(mUniqueID++);
	// Add to linear transform container
	mLinearTransformContainer.emplace_back();
	// Add to angular transform container
	mAngularTransformContainer.emplace_back();

	// TODO: Set up appropriate entity -> index and index -> entity maps. This is needed
	// in the case of removing bodies.
}

void P3DynamicsWorld::addRigidBody(
	float mass, 
	glm::vec3 const& position, 
	glm::vec3 const& velocity)
{
	mBodyContainer.emplace_back(mUniqueID++);

	mLinearTransformContainer.emplace_back();
	LinearTransform& lastLinearTransform = mLinearTransformContainer.back();
	lastLinearTransform.mass = mass;
	lastLinearTransform.inverseMass = 1.0f / mass;
	lastLinearTransform.position = position;
	lastLinearTransform.velocity = velocity;
	lastLinearTransform.momentum = mass * velocity;

	mMeshColliderContainer.emplace_back();
	mMeshColliderContainer.back().setModelMatrix(glm::translate(glm::mat4(1.0f), position));

	mAngularTransformContainer.emplace_back();
}

void P3DynamicsWorld::addRigidBody(LinearTransform const& linearTransform, AngularTransform const& angularTransform)
{
	mBodyContainer.emplace_back(mUniqueID++);

	mLinearTransformContainer.emplace_back(linearTransform);	// Copy constructor will be called here.

	// Add to angular transform container
	mAngularTransformContainer.emplace_back(angularTransform);
}

void P3DynamicsWorld::reset()
{
	mBodyContainer.clear();
	mLinearTransformContainer.clear();
	mAngularTransformContainer.clear();
	mMeshColliderContainer.clear();
	mUniqueID = 0u;
}

// TODO
void P3DynamicsWorld::fillWorldWithBodies()
{
	glm::vec3 position(0.0f);
	float mass = 0.0f;

	for (size_t i = 0; i < mMaxCapacity; ++i)
	{
		// Append unique ID
		mBodyContainer.emplace_back(mUniqueID);
		++mUniqueID;
	}
}

void P3DynamicsWorld::bowlingGameDemo()
{
	float startingX = -2.0f;

	std::vector<glm::vec4> vertices
	{
		glm::vec4{ -0.2f,  1.0f,  0.2f,  1.0f },
		glm::vec4{  0.2f,  1.0f,  0.2f,  1.0f },
		glm::vec4{  0.2f, -1.0f,  0.2f,  1.0f },
		glm::vec4{ -0.2f, -1.0f,  0.2f,  1.0f },

		glm::vec4{ -0.2f,  1.0f, -0.2f,  1.0f },
		glm::vec4{  0.2f,  1.0f, -0.2f,  1.0f },
		glm::vec4{  0.2f, -1.0f, -0.2f,  1.0f },
		glm::vec4{ -0.2f, -1.0f, -0.2f,  1.0f }
	};	// A skinny verson of a unit box

	for (float i = 0.0f; i < 5.0f; ++i)
	{
		addRigidBody(1, glm::vec3(startingX + i, 1.0f, -15.0f), glm::vec3(0.0f));
		mMeshColliderContainer.back().setVertices(vertices);
	}
}

void P3DynamicsWorld::stackingSpheresDemo()
{
	// Stack 2 unit spheres on top of each other. Same mass.
	addRigidBody(1, glm::vec3(0.0f, 4.0f, -20.0f), glm::vec3(0.0f));
	addRigidBody(1, glm::vec3(0.0f, 5.0f, -20.0f), glm::vec3(0.0f));
}

void P3DynamicsWorld::stackingBoxesDemo()
{
	// Stack 2 unit cubes on top of each other. Same mass.
	addRigidBody(1, glm::vec3(0.0f, 0.0f, -15.0f), glm::vec3(0.0f));
	addRigidBody(1, glm::vec3(0.0f, 5.0f, -15.0f), glm::vec3(0.0f));
}