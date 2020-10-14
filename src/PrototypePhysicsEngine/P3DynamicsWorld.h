/**
 * I guess inside this world we have physics enabled. Newton must be proud!
 * @author: Quan Bui
 * @version: 04/09/2020
 */

#pragma once

#ifndef P3_DYNAMICS_WORLD_H
#define P3_DYNAMICS_WORLD_H

#include <glm/vec3.hpp>
#include <vector>
#include <unordered_map>
#include <memory>

#include "RigidBody.h"
#include "P3BroadPhaseCollisionDetection.h"
#include "P3NarrowPhaseCollisionDetection.h"
#include "P3Transform.h"
#include "P3ConstraintSolver.h"
#include "P3Integrator.h"

using LinearTransformContainerPtr = std::shared_ptr<std::vector<LinearTransform>>;

class P3DynamicsWorld
{
public:
	P3DynamicsWorld() {}
	P3DynamicsWorld(size_t maxCapacity) : mMaxCapacity(maxCapacity) {}	// Might want error checking here

	//---------------------------- Core ----------------------------//
	void stepSimulation(double dt);

	//---------------------- Add bodies to the world ----------------------//
	// Is it the world responsibility to check for max capacity before adding?
	void addRigidBody();
	void addRigidBody(float, glm::vec3 const&, glm::vec3 const&);
	void addRigidBody(LinearTransform const&, AngularTransform const&);

	//------------------------ Demos ------------------------//
	void fillWorldWithBodies();
	void bowlingGameDemo();
	void stackingSpheresDemo();
	void stackingBoxesDemo();

	//----------------------- Some getters and setters -----------------------//
	inline float getGravity() const { return mGravity; }
	inline float getAirDrag() const { return mAirDrag; }
	inline size_t getOccupancy() const { return mLinearTransformContainer.size(); }
	inline size_t getMaxCapacity() const { return mMaxCapacity; }

	inline std::vector<LinearTransform> const& getLinearTransformContainer() const
	{
		return mLinearTransformContainer;
	}

	void setGravity(float gravity) { mGravity = gravity; }
	void setMaxCapacity(const int maxCapacity) { mMaxCapacity = maxCapacity; }	// Need error checking

	inline bool isFull() { return mBodyContainer.size() == mMaxCapacity; }

private:
	//---------------- Constant physics quantities ----------------//
	float mGravity{ 0.001f }, mAirDrag{ 2.0f };
	size_t mMaxCapacity{ 20 };
	float tickRate{ 0.0167f };	// 1/60

	//------------------------- Entity list -------------------------//
	std::vector<RigidBody> mBodyContainer;
	RigidBody mUniqueID = 0u;

	//----------------------- Component list -----------------------//
	std::vector<LinearTransform> mLinearTransformContainer;
	std::vector<AngularTransform> mAngularTransformContainer;

	//----------------- Map of index to rigid body -----------------//
	// @reference: https://austinmorlan.com/posts/entity_component_system/
	std::unordered_map<RigidBody, size_t> mEntityToIndexMap;
	std::unordered_map<size_t, RigidBody> mIndexToEntityMap;

	//--------------------- Physics pipeline ---------------------//
	// Order of operations for each timestep: Collision -> apply forces -> solve constraints -> update positions 
	P3OpenGLComputeBroadPhase broadPhase;
	//P3OpenGLComputeNarrowPhase narrowPhase;
	P3ConstraintSolver constraintSolver;	// Produces forces to make sure things don't phase past each other
	P3Integrator integrator;				// Actually integrate the force vector and apply to linear transform
};

//class P3DynamicsWorld
//{
//public:
//	P3DynamicsWorld() {}
//	P3DynamicsWorld(size_t maxCapacity) : mMaxCapacity(maxCapacity) {}	// Might want error checking here
//
//	//----------------------- Some getters and setters -----------------------
//	float getGravity() const { return mGravity; }
//	float getAirDrag() const { return mAirDrag; }
//	size_t getMaxCapacity() const { return mMaxCapacity; }
//
//	void setGravity(float gravity) { mGravity = gravity; }
//	void setGravityToBody(DynamicBody& body) { body.setGravity(glm::vec3(0.f, -mGravity, 0.f)); }
//	void setMaxCapacity(const int maxCapacity) { mMaxCapacity = maxCapacity; }	// Need error checking
//
//	//---------------------- Add bodies to the world ----------------------
//	bool addDynamicBody(glm::vec3 const& position, BoundingVolume* bound, float bounce = 0.f, float mass = 1.f);
//	bool addStaticBody(glm::vec3 const& position, BoundingVolume* bound, float bounce = 0.f);
//	bool addKinematicBody(glm::vec3 const& position, BoundingVolume* bound, float bounce = 0.f);
//
//	bool isFull() { return mRigidBodyPtrContainer.size() == mMaxCapacity; }
//
//	// Broad phase AABB style. This could be expanded a whole lot, with complicated trees
//	bool checkCollision(DynamicBody const&, DynamicBody const&);
//	bool checkCollision(DynamicBody const&, StaticBody const&);
//
//	// Should be able to modify the bodies, such as their linear velocity
//	void resolveCollision(DynamicBody&, DynamicBody&);
//	void resolveCollision(DynamicBody&, StaticBody&);
//	void resolveCollision(DynamicBody&, KinematicBody&);
//
//private:
//	float mGravity{ 0.001f }, mAirDrag{ 2.0f };
//	size_t mMaxCapacity{ 20 };
//	std::vector<RigidBody*> mRigidBodyPtrContainer;	// How to identify a body in this container??
//};

#endif // P3_DYNAMICS_WORLD_H