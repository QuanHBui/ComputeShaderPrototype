/**
 * Implementation of the DynamicsWorld
 * For now just check collision between 2 Dynamic bodies
 * @author: Quan Bui
 * @version: 10/15/2020
 */

#include "P3DynamicsWorld.h"

#include <ctime>
#include <iostream>

#include <glm/gtx/transform.hpp>

#include "P3Simplex.h"
#include "P3Sat.h"

float randf()
{
	return rand() / float(RAND_MAX);
}

void P3DynamicsWorld::init()
{
	mBroadPhase.init();
	mNarrowPhase.init(mBroadPhase.getBoxCollidersID(), mBroadPhase.getCollisionPairsID());
	mConstraintSolver.init();
}

void P3DynamicsWorld::detectCollisions()
{
	mBroadPhase.step(mBoxColliderContainer);

#ifdef NARROW_PHASE_CPU
	BoxColliderGpuPackage boxColliderPkg;
	for (int i = 0; i < mBoxColliderContainer.size(); ++i)
	{
		for (int j = 0; j < cBoxColliderVertCount; ++j)
		{
			boxColliderPkg.boxColliders[i][j] = mBoxColliderContainer[i].mVertices[j];
		}
	}

	mManifoldPkg = P3Sat(boxColliderPkg, mBroadPhase.getPCollisionPairPkg());
#else
	mNarrowPhase.step();
	mManifoldPkg = *mNarrowPhase.getPManifoldPkg(); // Warning: Unnecessary copying here
#endif
}

void P3DynamicsWorld::updateMultipleBoxes(float dt)
{
	static float radians = 0.0f;

	for (int i = 0; i < 50; ++i)
	{
		mRigidLinearTransformContainer[i].velocity.x += dt * cosf(10.0f * radians);
		mRigidLinearTransformContainer[i].velocity.y += dt * cosf(10.0f * radians);
		mRigidLinearTransformContainer[i].velocity.z += dt * cosf(10.0f * radians);

		mRigidLinearTransformContainer[i].momentum    = mRigidLinearTransformContainer[i].velocity * mRigidLinearTransformContainer[i].mass;
		mRigidLinearTransformContainer[i].position   += mRigidLinearTransformContainer[i].velocity * float(dt);
	}

	for (int j = 50; j < 100; ++j)
	{
		mRigidLinearTransformContainer[j].velocity.x += dt * cosf(10.0f * radians);
		mRigidLinearTransformContainer[j].velocity.y += dt * cosf(10.0f * radians);
		mRigidLinearTransformContainer[j].velocity.z += dt * cosf(10.0f * radians);

		mRigidLinearTransformContainer[j].momentum    = mRigidLinearTransformContainer[j].velocity * mRigidLinearTransformContainer[j].mass;
		mRigidLinearTransformContainer[j].position   += mRigidLinearTransformContainer[j].velocity * float(dt);
	}

	for (unsigned int i = 0; i < mRigidLinearTransformContainer.size(); ++i)
		mMeshColliderContainer[i].update(glm::translate(mRigidLinearTransformContainer[i].position));

	for (unsigned int i = 0; i < mRigidLinearTransformContainer.size(); ++i)
		mBoxColliderContainer[i].update(glm::translate(mRigidLinearTransformContainer[i].position));

	radians += 1.0f;
}

 // Order of operations for each timestep: Collision -> apply forces -> solve constraints -> update positions
void P3DynamicsWorld::updateBowlingGame(float dt)
{
	// To define a plane, we need a normal and a point
	glm::vec3 surfaceNormal{ 0.0f, 1.0f, 0.0f };
	glm::vec3 surfacePoint{ 0.0f, 1.0f, 0.0f };

	std::vector<glm::vec3> sampleVelocityContainer;

	for (int rigidBodyID : mBodyContainer)
	{
		LinearTransform &linearTransform = mRigidLinearTransformContainer[rigidBodyID];

		glm::vec3 accumulateImpulse{ 0.0f };
		glm::vec3 sampleVelocity = linearTransform.velocity;
		glm::vec3 samplePosition = linearTransform.position;

		// Collision detection - broad phase + near phase
		bool hasCollided = false;
		if (rigidBodyID < 5u && getOccupancy() > 5u)
		{
			for (unsigned int i = 5u; i < getOccupancy(); ++i)
			{
				// A very very terrible broad phase
				if (glm::length(mRigidLinearTransformContainer[i].position - linearTransform.position) <= 1.25f)
				{
					// A very very terrible narrow phase
					P3Simplex gjkSimplex;
					hasCollided = P3Gjk(mMeshColliderContainer[rigidBodyID], mMeshColliderContainer[i], gjkSimplex);

					// A very very terrible collision resolution
					if (hasCollided)
					{
						P3Epa(mMeshColliderContainer[rigidBodyID], mMeshColliderContainer[i], gjkSimplex);

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
		mRigidLinearTransformContainer[i].velocity  = *sampleVelocityContainerIter;
		mRigidLinearTransformContainer[i].momentum  = *sampleVelocityContainerIter * mRigidLinearTransformContainer[i].mass;
		mRigidLinearTransformContainer[i].position += *sampleVelocityContainerIter * float(dt);
	}

	for (unsigned int i = 0; i < mRigidLinearTransformContainer.size(); ++i)
		mMeshColliderContainer[i].update(glm::translate(mRigidLinearTransformContainer[i].position));

	for (unsigned int i = 0; i < mRigidLinearTransformContainer.size(); ++i)
		mBoxColliderContainer[i].update(glm::translate(mRigidLinearTransformContainer[i].position));
}

// Specifically for the controllable box demo
void P3DynamicsWorld::updateControllableBox(float dt, glm::vec3 const &deltaP)
{
	// 1st box is static, 2nd box is kinematic/controllable.
	mRigidLinearTransformContainer[1].position += deltaP;

	glm::mat4 extraTransforms = glm::rotate(0.785f, glm::vec3(0.0f, 1.0f, 0.0f));
	// TODO: This is extremely ad hoc, please fix!
	glm::mat4 temp = glm::translate(mRigidLinearTransformContainer[1].position) * extraTransforms;
	mMeshColliderContainer[1].update(temp);
	mBoxColliderContainer[1].update(temp);

	float static angle = 0.0f;
	// Box 4 is scaled non-uniformly and rotating.
	glm::mat4 scale     = glm::scale(glm::vec3(3.0f, 1.0f, 3.0f));
	glm::mat4 rotate    = glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f));
	rotate = glm::mat4(1.0f);
	glm::mat4 translate = glm::translate(mRigidLinearTransformContainer[3].position);
	glm::mat4 ctm       = translate * rotate * scale;
	mMeshColliderContainer[3].update(ctm);
	mBoxColliderContainer[3].update(ctm);

	angle += dt * 0.52f;
	angle  = angle >= 6.28f ? 0.0f : angle; // This is so that we won't get floating point overflow.
}

void P3DynamicsWorld::updateGravityTest(float dt)
{
	// Apply forces
	for (LinearTransform &linearTransform : mRigidLinearTransformContainer)
	{
		linearTransform.velocity.y -= 4.0f * dt;
	}

	// Solve constraints - produces final impulses at certain contact points
#ifdef NARROW_PHASE_CPU
	mConstraintSolver.solve(
		mManifoldPkg,
		mBoxColliderContainer,
		mRigidLinearTransformContainer,
		mRigidAngularTransformContainer
	);
#else
	// TODO: Use the implementation on the GPU
	mConstraintSolver.solve(
		mManifoldPkg,
		mBoxColliderContainer,
		mRigidLinearTransformContainer,
		mRigidAngularTransformContainer
	);
#endif

	std::vector<glm::vec3> const &solveLinearImpulseContainer  = mConstraintSolver.getLinearImpulseContainer();
	std::vector<glm::vec3> const &solveAngularImpulseContainer = mConstraintSolver.getAngularImpulseContainer();

	// Apply final linear transforms
	// There must be a better way to connect linear transforms and hit boxes.
	// First wipe - linear transforms
	for (int i = 0; i < mRigidLinearTransformContainer.size(); ++i)
	{
		LinearTransform &linearTransform = mRigidLinearTransformContainer[i];
		linearTransform.velocity += solveLinearImpulseContainer[i];

		//if (solveLinearImpulseContainer[i] != glm::vec3(0.0f))
		//	linearTransform.velocity = glm::vec3(0.0f);

		linearTransform.position += dt * linearTransform.velocity;

		mBoxColliderCtmContainer[i] = glm::translate(linearTransform.position);
	}

	// Apply final angular transforms
	for (int j = 0; j < mRigidAngularTransformContainer.size(); ++j)
	{
		AngularTransform &angularTransform = mRigidAngularTransformContainer[j];
		angularTransform.angularVelocity  += solveAngularImpulseContainer[j];

		//if (solveAngularImpulseContainer[j] != glm::vec3(0.0f))
		//	angularTransform.angularVelocity = glm::vec3(0.0f);

		angularTransform.tempOrientation  += dt * glm::length(angularTransform.angularVelocity);

		if (angularTransform.angularVelocity == glm::vec3(0.0f))
		{
			mBoxColliderCtmContainer[j] *= glm::mat4(1.0f);
		}
		else
		{
			mBoxColliderCtmContainer[j] *= glm::rotate(angularTransform.tempOrientation, glm::normalize(angularTransform.angularVelocity));
		}
	}

	for (int m = 0; m < mBoxColliderCtmContainer.size(); ++m)
	{
		mBoxColliderContainer[m].update(mBoxColliderCtmContainer[m]);
	}
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

	return glm::vec3(0.0f);
}

// TODO: WIP
int P3DynamicsWorld::addRigidBody()
{
	// Generate unique ID and add to ID container
	mBodyContainer.emplace_back(mUniqueID++);
	// Add to linear transform container
	mRigidLinearTransformContainer.emplace_back();
	// Add to angular transform container
	mRigidAngularTransformContainer.emplace_back();

	// TODO: Set up appropriate entity -> index and index -> entity maps. This is needed
	// in the case of removing bodies.

	return mUniqueID;
}

int P3DynamicsWorld::addRigidBody(float mass, glm::vec3 const &position, glm::vec3 const &velocity)
{
	mBodyContainer.emplace_back(mUniqueID++);

	mRigidLinearTransformContainer.emplace_back();

	LinearTransform &lastLinearTransform = mRigidLinearTransformContainer.back();
	lastLinearTransform.mass = mass;
	lastLinearTransform.inverseMass = 1.0f / mass;
	lastLinearTransform.position = position;
	lastLinearTransform.velocity = velocity;
	lastLinearTransform.momentum = mass * velocity;

	mMeshColliderContainer.emplace_back();
	mMeshColliderContainer.back().update(glm::translate(position));

	// For Gpu collision detection
	mBoxColliderContainer.emplace_back();
	mBoxColliderContainer.back().update(glm::translate(position));

	mRigidAngularTransformContainer.emplace_back();

	mBoxColliderCtmContainer.emplace_back(glm::translate(position));

	return mUniqueID;
}

int P3DynamicsWorld::addRigidBody( float mass, glm::vec3 const &position, glm::vec3 const &velocity,
								   glm::mat4 const &extraTransforms )
{
	mBodyContainer.emplace_back(mUniqueID++);

	mRigidLinearTransformContainer.emplace_back();

	LinearTransform &lastLinearTransform = mRigidLinearTransformContainer.back();
	lastLinearTransform.mass = mass;
	lastLinearTransform.inverseMass = 1.0f / mass;
	lastLinearTransform.position = position;
	lastLinearTransform.velocity = velocity;
	lastLinearTransform.momentum = mass * velocity;

	mMeshColliderContainer.emplace_back();
	mMeshColliderContainer.back().update(glm::translate(position) * extraTransforms);

	// For Gpu collision detection
	mBoxColliderContainer.emplace_back();
	mBoxColliderContainer.back().update(glm::translate(position) * extraTransforms);

	mRigidAngularTransformContainer.emplace_back();

	mBoxColliderCtmContainer.emplace_back();

	return mUniqueID;
}

// TODO: WIP
int P3DynamicsWorld::addRigidBody(LinearTransform const &linearTransform, AngularTransform const &angularTransform)
{
	mBodyContainer.emplace_back(mUniqueID++);

	mRigidLinearTransformContainer.emplace_back(linearTransform); // Copy constructor will be called here.

	// Add to angular transform container
	mRigidAngularTransformContainer.emplace_back(angularTransform);

	return mUniqueID;
}

// A static body can do more than just standing still, but for now that's all it does.
int P3DynamicsWorld::addStaticBody(glm::vec3 const &position)
{
	mBodyContainer.emplace_back(mUniqueID++);

	mStaticLinearTransformContainer.emplace_back();

	LinearTransform &lastLinearTransform = mStaticLinearTransformContainer.back();
	lastLinearTransform.mass = std::numeric_limits<float>::max();
	lastLinearTransform.inverseMass = 0.0f;
	lastLinearTransform.position = position;
	lastLinearTransform.velocity = glm::vec3(0.0f);
	lastLinearTransform.momentum = glm::vec3(std::numeric_limits<float>::max());

	mMeshColliderContainer.emplace_back();
	mMeshColliderContainer.back().update(glm::translate(position));

	// For Gpu collision detection
	mBoxColliderContainer.emplace_back();
	mBoxColliderContainer.back().update(glm::translate(position));

	mStaticAngularTransformContainer.emplace_back();

	mBoxColliderCtmContainer.emplace_back(glm::translate(position));

	return mUniqueID;
}

int P3DynamicsWorld::addStaticBodies(std::vector<glm::vec3> const &posContainer)
{
	mBodyContainer.emplace_back(mUniqueID++);
	return mUniqueID;
}

void P3DynamicsWorld::reset()
{
	mBodyContainer.clear();
	mRigidLinearTransformContainer.clear();
	mRigidAngularTransformContainer.clear();
	mMeshColliderContainer.clear();
	mBoxColliderContainer.clear();
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

		glm::mat4 translateMatrix = glm::translate(glm::vec3(startingX + i, 1.0f, -15.0f));
		mMeshColliderContainer.back().update(translateMatrix);

		mBoxColliderContainer.back().setInstanceVertices(vertices.data());
		mBoxColliderContainer.back().update(translateMatrix);
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

		glm::mat4 translateMatrix = glm::translate(2.0f * glm::vec3(x, y, z));
		mMeshColliderContainer.back().update(translateMatrix);

		mBoxColliderContainer.back().update(translateMatrix);
	}
}

void P3DynamicsWorld::controllableBoxDemo()
{
	addRigidBody(1.0f, glm::vec3(-6.0f, -2.0f, 5.0f), glm::vec3(0.0f)); // The static box
	addRigidBody(1.0f, glm::vec3( 0.0f, -2.0f, 7.0f), glm::vec3(0.0f),
				 glm::rotate(0.785f, glm::vec3(0.0f, 1.0f, 0.0f))); // The kinematic box
	addRigidBody(1.0f, glm::vec3( 6.0f, -2.0f, 5.0f), glm::vec3(0.0f)); // Another static box
	addRigidBody(1.0f, glm::vec3( 0.0f, -2.0f, 5.0f), glm::vec3(0.0f)); // Static box with different size and rotating
}
