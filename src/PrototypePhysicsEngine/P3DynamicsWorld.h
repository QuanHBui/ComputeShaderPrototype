/**
 * I guess inside this world we have physics enabled. Newton must be proud!
 * @author: Quan Bui
 */

#pragma once

#ifndef P3_DYNAMICS_WORLD_H
#define P3_DYNAMICS_WORLD_H

#include <glm/vec3.hpp>
#include <vector>
#include <unordered_map>
#include <memory>

#include "P3BroadPhaseCollisionDetection.h"
#include "P3Collider.h"
#include "P3ConstraintSolver.h"
#include "P3Integrator.h"
#include "P3NarrowPhaseCollisionDetection.h"
#include "P3NarrowPhaseCommon.h"
#include "P3Transform.h"

#define BROAD_PHASE_CPU
#define NARROW_PHASE_CPU

using LinearTransformContainerPtr = std::shared_ptr<std::vector<LinearTransform>>;

class P3DynamicsWorld
{
public:
	P3DynamicsWorld() {}
	P3DynamicsWorld(size_t maxCapacity) : mMaxCapacity(maxCapacity) {} // Might want error checking here

	void init();

	void detectCollisions();

	void updateMultipleBoxes(float dt);
	void updateBowlingGame(float dt);
	void updateControllableBox(float dt, glm::vec3 const &);
	void updateGravityTest(float dt);

	glm::vec3 castRay(glm::vec3 const &, glm::vec3 const &);

	//---------------------- Add bodies to the world ----------------------//
	// Is it the world responsibility to check for max capacity before adding?
	int addRigidBody();
	int addRigidBody(float, glm::vec3 const &, glm::vec3 const &);
	int addRigidBody(float, glm::vec3 const &, glm::vec3 const &, glm::mat4 const &);
	int addRigidBody(LinearTransform const &, AngularTransform const &);

	int addStaticBody(glm::vec3 const &);
	int addStaticBodies(std::vector<glm::vec3> const &);

	//------------------------ Demos ------------------------//
	void reset();
	void fillWorldWithBodies();
	void bowlingGameDemo();
	void stackingSpheresDemo();
	void stackingBoxesDemo();
	void multipleBoxesDemo();
	void controllableBoxDemo();

	//----------------------- Some getters and setters -----------------------//
	float getGravity() const { return mGravity; }
	float getAirDrag() const { return mAirDrag; }
	unsigned int getOccupancy() const { return mBodyContainer.size(); }
	unsigned int getNumBoxColliders() const { return mBoxColliderContainer.size(); }
	unsigned int getMaxCapacity() const { return mMaxCapacity; }
	std::vector<P3BoxCollider> const &getBoxColliders() const { return mBoxColliderContainer; }

	std::vector<LinearTransform> const &getRigidLinearTransformContainer() const
	{
		return mRigidLinearTransformContainer;
	}

	std::vector<LinearTransform> const &getStaticLinearTransformContainer() const
	{
		return mStaticLinearTransformContainer;
	}

	std::vector<AngularTransform> const &getRigidAngularTransformContainer() const
	{
		return mRigidAngularTransformContainer;
	}

	std::vector<AngularTransform> const &getStaticAngularTransformContainer() const
	{
		return mStaticAngularTransformContainer;
	}

	CollisionPairGpuPackage const *getPCollisionPairPkg() const
	{
#ifdef BROAD_PHASE_CPU
		return mCpuBroadPhase.getPCollisionPkg();
#else
		return mBroadPhase.getPCollisionPairPkg();
#endif // BROAD_PHASE_CPU
	}

	ManifoldGpuPackage const *getPManifoldPkg() const
	{
#ifdef NARROW_PHASE_CPU
		return &mManifoldPkg;
#else
		return mNarrowPhase.getPManifoldPkg();
#endif // NARROW_PHASE_CPU
	}

	void setGravity(float gravity) { mGravity = gravity; }
	void setMaxCapacity(const int maxCapacity) { mMaxCapacity = maxCapacity; } // Need error checking

	bool isFull() { return mBodyContainer.size() >= mMaxCapacity; }

private:
	//---------------- Constant physics quantities ----------------//
	float mGravity{ 0.001f }, mAirDrag{ 2.0f };
	size_t mMaxCapacity{ 20u };

	//------------------------- Entity list -------------------------//
	std::vector<int> mBodyContainer;
	int mUniqueID = 0u;

	//----------------------- Component list -----------------------//
	std::vector<LinearTransform> mRigidLinearTransformContainer;
	std::vector<AngularTransform> mRigidAngularTransformContainer;
	std::vector<LinearTransform> mStaticLinearTransformContainer;
	std::vector<AngularTransform> mStaticAngularTransformContainer;
	std::vector<P3MeshCollider> mMeshColliderContainer;
	std::vector<P3BoxCollider> mBoxColliderContainer;
	std::vector<glm::mat4> mBoxColliderCtmContainer;

	//----------------- Data package optimized for the GPU -----------------//
	LinearTransformGpuPackage mLinearTransformPkg; // For rigid and kinematic bodies
	ManifoldGpuPackage mManifoldPkg;

	//----------------- Map of index to rigid body -----------------//
	// @reference: https://austinmorlan.com/posts/entity_component_system/
	//std::unordered_map<int, int> mEntityToIndexMap;
	//std::unordered_map<int, int> mIndexToEntityMap;

	//--------------------- Physics pipeline ---------------------//
	// Order of operations for each timestep: Collision -> apply forces -> solve constraints -> update positions
	P3OpenGLComputeBroadPhase mBroadPhase;
	P3::CpuBroadPhase mCpuBroadPhase;

	P3OpenGLComputeNarrowPhase mNarrowPhase;

	P3ConstraintSolver mConstraintSolver; // Produces forces to make sure things don't phase past each other
	P3Integrator mIntegrator;             // Actually integrates the force vector and apply to linear transform
};

#endif // P3_DYNAMICS_WORLD_H
