/**
 * Implementation of the DynamicsWorld
 * For now just check collision between 2 Dynamic bodies
 * @author: Quan Bui
 * @version: 10/15/2020
 */

#include "P3DynamicsWorld.h"

#include <ctime>
#include <iostream>

#include "P3Simplex.h"

float randf()
{
	return rand() / float(RAND_MAX);
}

void P3DynamicsWorld::init()
{
	broadPhase.init();
	narrowPhase.init(broadPhase.getBoxCollidersID(), broadPhase.getCollisionPairsID());
}

CollisionPairGpuPackage const &P3DynamicsWorld::update(double dt)
{
	mCollisionPairCpuData = std::move(broadPhase.step(mBoxColliders));

	narrowPhase.step(mBoxColliders.size());

	static float radians = 0.0f;

	for (int i = 0; i < 50; ++i)
	{
		mLinearTransformContainer[i].velocity.x += dt * cosf(10.0f * radians);
		mLinearTransformContainer[i].velocity.y += dt * cosf(10.0f * radians);
		mLinearTransformContainer[i].velocity.z += dt * cosf(10.0f * radians);

		mLinearTransformContainer[i].momentum    = mLinearTransformContainer[i].velocity * mLinearTransformContainer[i].mass;
		mLinearTransformContainer[i].position   += mLinearTransformContainer[i].velocity * float(dt);
	}

	for (int j = 50; j < 100; ++j)
	{
		mLinearTransformContainer[j].velocity.x += dt * cosf(10.0f * radians);
		mLinearTransformContainer[j].velocity.y += dt * cosf(10.0f * radians);
		mLinearTransformContainer[j].velocity.z += dt * cosf(10.0f * radians);

		mLinearTransformContainer[j].momentum    = mLinearTransformContainer[j].velocity * mLinearTransformContainer[j].mass;
		mLinearTransformContainer[j].position   += mLinearTransformContainer[j].velocity * float(dt);
	}

	for (unsigned int i = 0; i < mLinearTransformContainer.size(); ++i)
		mMeshColliderContainer[i].update(glm::translate(glm::mat4(1.0f), mLinearTransformContainer[i].position));

	for (unsigned int i = 0; i < mLinearTransformContainer.size(); ++i)
		mBoxColliders[i].update(glm::translate(glm::mat4(1.0f), mLinearTransformContainer[i].position));

	radians += 1.0f;

	return mCollisionPairCpuData;
}

 // Order of operations for each timestep: Collision -> apply forces -> solve constraints -> update positions
CollisionPairGpuPackage const &P3DynamicsWorld::updateAndResolve(double dt)
{
	// To define a plane, we need a normal and a point
	glm::vec3 surfaceNormal{ 0.0f, 1.0f, 0.0f };
	glm::vec3 surfacePoint{ 0.0f, 1.0f, 0.0f };

	std::vector<glm::vec3> sampleVelocityContainer;

	mCollisionPairCpuData = std::move(broadPhase.step(mBoxColliders));
	std::cout << mCollisionPairCpuData.collisionPairs[0].x << ", "
		<< mCollisionPairCpuData.collisionPairs[0].y << std::endl;

	for (RigidBody const &rigidBody : mBodyContainer)
	{
		LinearTransform &linearTransform = mLinearTransformContainer[rigidBody];

		glm::vec3 accumulateImpulse{ 0.0f };
		glm::vec3 sampleVelocity = linearTransform.velocity;
		glm::vec3 samplePosition = linearTransform.position;

		// Collision detection - broad phase + near phase
		bool hasCollided = false;
		if (rigidBody < 5u && getOccupancy() > 5u)
		{
			for (unsigned int i = 5u; i < getOccupancy(); ++i)
			{
				// A very very terrible broad phase
				if (glm::length(mLinearTransformContainer[i].position - linearTransform.position) <= 1.25f)
				{
					// A very very terrible narrow phase
					P3Simplex gjkSimplex;
					hasCollided = P3Gjk(mMeshColliderContainer[rigidBody], mMeshColliderContainer[i], gjkSimplex);

					// A very very terrible collision resolution
					if (hasCollided)
					{
						P3Epa(mMeshColliderContainer[rigidBody], mMeshColliderContainer[i], gjkSimplex);

						// Response impulse
						//accumulateImpulse += mLinearTransformContainer[i].momentum;
						//std::cout << "Collided!" << std::endl;
					}
				}
			}
		}

		// Apply forces - gravity most likely
		sampleVelocity.y -= 9.81f * dt;

		// Check for constraint and solve it
		// MULTIPLE ITERATIONS
		// NO BOUNCE!
		int iterations = 4;
		while (iterations--)
		{
			// Update position of rigid body after apply forces
			sampleVelocity += accumulateImpulse * linearTransform.inverseMass;
			samplePosition += sampleVelocity * float(dt);

			// Express the constraint in term of position. This is a position constraint.
			if (samplePosition.y - (-3.0f) < 0.0f)
			{
				// We have to modify the accumulate velocity to solve this position constraint
				// How much in the y direction that we have to push the object, this y direction can be generalize
				//  to just the surface/plane normal.
				// THE IMPULSE CAN PUSH, BUT NOT PULL.
				float signedDistance = samplePosition.y - (-3.0f);

				samplePosition.y -= signedDistance;
				linearTransform.position = samplePosition;
				// Make current velocity zero
				accumulateImpulse += glm::abs(sampleVelocity.y) * glm::normalize(glm::vec3(0.0f, -signedDistance, 0.0f));
			}
		}

		// Apply the final impulse
		sampleVelocity = glm::vec3(linearTransform.velocity) + accumulateImpulse * linearTransform.inverseMass;
		sampleVelocity.y -= 9.81f * dt;
		sampleVelocityContainer.emplace_back(sampleVelocity);
	}

	std::vector<glm::vec3>::iterator sampleVelocityContainerIter;
	for ( sampleVelocityContainerIter  = sampleVelocityContainer.begin()
		; sampleVelocityContainerIter != sampleVelocityContainer.end()
		; sampleVelocityContainerIter++)
	{
		size_t i = std::distance(sampleVelocityContainer.begin(), sampleVelocityContainerIter);
		mLinearTransformContainer[i].velocity  = *sampleVelocityContainerIter;
		mLinearTransformContainer[i].momentum  = *sampleVelocityContainerIter * mLinearTransformContainer[i].mass;
		mLinearTransformContainer[i].position += *sampleVelocityContainerIter * float(dt);
	}

	for (unsigned int i = 0; i < mLinearTransformContainer.size(); ++i)
		mMeshColliderContainer[i].update(glm::translate(glm::mat4(1.0f), mLinearTransformContainer[i].position));

	for (unsigned int i = 0; i < mLinearTransformContainer.size(); ++i)
		mBoxColliders[i].update(glm::translate(glm::mat4(1.0f), mLinearTransformContainer[i].position));

	return mCollisionPairCpuData;
}

// Specifically for the controllable box demo
CollisionPairGpuPackage const &P3DynamicsWorld::update(double dt, glm::vec3 const &deltaP)
{
	mCollisionPairCpuData = std::move(broadPhase.step(mBoxColliders));
	narrowPhase.step(mBoxColliders.size());

	// 1st box is static, 2nd box is kinematic/controllable.
	mLinearTransformContainer[1].position += deltaP;
	mMeshColliderContainer[1].update(glm::translate(glm::mat4(1.0f), mLinearTransformContainer[1].position));
	mBoxColliders[1].update(glm::translate(glm::mat4(1.0f), mLinearTransformContainer[1].position));

	float static angle = 0.0f;
	// Box 3 is scaled non-uniformly and rotating.
	glm::mat4 scale     = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 1.0f, 1.0f));
	glm::mat4 rotate    = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), mLinearTransformContainer[3].position);
	glm::mat4 ctm       = translate * rotate * scale;
	mMeshColliderContainer[3].update(ctm);
	mBoxColliders[3].update(ctm);

	angle += dt * 0.52f;
	angle  = angle >= 6.28f ? 0.0f : angle; // This is so that we won't get floating point overflow.

	return mCollisionPairCpuData;
}

// TODO: WIP
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

glm::vec3 P3DynamicsWorld::castRay(glm::vec3 const &start, glm::vec3 const &direction)
{
	// Define where the plane of interaction will be
	glm::vec3 planePoint{ 0.0f, 0.0f, -15.0f };
	glm::vec3 planeNormal = start - planeNormal;

	// Check if ray direction is perpendicular to plane normal
	float dDotN = glm::dot(direction, planeNormal);

	if (dDotN <= 0.0001f)
		return glm::vec3{ 0.0f }; // Prob there's a better value to return

	// Discard any t that is negative or close to 0.0f
	// -15.0f is the signed distance from world origin.
	float t = (-15.0f - glm::dot(start, planeNormal) / dDotN);

	if (t > 0.0001f)
		return direction * t + start;
}

void P3DynamicsWorld::addRigidBody(float mass, glm::vec3 const &position, glm::vec3 const &velocity)
{
	mBodyContainer.emplace_back(mUniqueID++);

	mLinearTransformContainer.emplace_back();

	LinearTransform &lastLinearTransform = mLinearTransformContainer.back();
	lastLinearTransform.mass = mass;
	lastLinearTransform.inverseMass = 1.0f / mass;
	lastLinearTransform.position = position, mBodyContainer.back();
	lastLinearTransform.velocity = velocity, mBodyContainer.back();
	lastLinearTransform.momentum = mass * velocity, mBodyContainer.back();

	mMeshColliderContainer.emplace_back();
	mMeshColliderContainer.back().update(glm::translate(glm::mat4(1.0f), position));

	// For Gpu collision detection
	mBoxColliders.emplace_back();
	mBoxColliders.back().update(glm::translate(glm::mat4(1.0f), position));

	mAngularTransformContainer.emplace_back();
}

// TODO: WIP
void P3DynamicsWorld::addRigidBody(LinearTransform const &linearTransform, AngularTransform const &angularTransform)
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
	mBoxColliders.clear();
	mUniqueID = 0u;
}

// TODO:
void P3DynamicsWorld::fillWorldWithBodies()
{
	glm::vec3 position(0.0f);
	float mass = 1.0f;

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
		addRigidBody(1.0f, glm::vec3(startingX + i, 1.0f, -15.0f), glm::vec3(0.0f));
		mMeshColliderContainer.back().setInstanceVertices(vertices);

		glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(startingX + i, 1.0f, -15.0f));
		mMeshColliderContainer.back().update(translateMatrix);

		mBoxColliders.back().setInstanceVertices(vertices.data());
		mBoxColliders.back().update(translateMatrix);
	}
}

void P3DynamicsWorld::stackingSpheresDemo()
{
	// Stack 2 unit spheres on top of each other. Same mass.
	addRigidBody(1.0f, glm::vec3(0.0f, 4.0f, -20.0f), glm::vec3(0.0f));
	addRigidBody(1.0f, glm::vec3(0.0f, 5.0f, -20.0f), glm::vec3(0.0f));
}

void P3DynamicsWorld::stackingBoxesDemo()
{
	// Stack 2 unit cubes on top of each other. Same mass.
	addRigidBody(1.0f, glm::vec3(0.0f, 0.0f, -15.0f), glm::vec3(0.0f));
	addRigidBody(1.0f, glm::vec3(0.0f, 5.0f, -15.0f), glm::vec3(0.0f));
}

void P3DynamicsWorld::multipleBoxesDemo()
{
	glm::vec3 startingPosition;
	float x, y, z;
	float r, g, b;

	x = y = z = 0.0f;
	r = g = b = 0.0f;

	// Spawn multiple boxes randomly in the world
	for (int i = 0; i < 100; ++i)
	{
		x = randf() * 10.0f - 5.0f;
		y = randf() * 10.0f - 5.0f;
		z = randf() * 10.0f - 5.0f;

		r = randf() *  2.0f - 1.0f;
		g = randf() *  2.0f - 1.0f;
		b = randf() *  2.0f - 1.0f;

		addRigidBody(1.0f, 2.0f * glm::vec3(x, y, z), glm::vec3(r, g, b));

		glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), 2.0f * glm::vec3(x, y, z));
		mMeshColliderContainer.back().update(translateMatrix);

		mBoxColliders.back().update(translateMatrix);
	}
}

void P3DynamicsWorld::controllableBoxDemo()
{
	addRigidBody(1.0f, glm::vec3(-6.0f, -2.0f, 5.0f), glm::vec3(0.0f)); // The static box
	addRigidBody(1.0f, glm::vec3( 0.0f, -2.0f, 7.0f), glm::vec3(0.0f)); // The kinematic box
	addRigidBody(1.0f, glm::vec3( 6.0f, -2.0f, 5.0f), glm::vec3(0.0f)); // Another static box
	addRigidBody(1.0f, glm::vec3( 0.0f, -2.0f, 5.0f), glm::vec3(0.0f)); // Static box with difference size and rotating
}
