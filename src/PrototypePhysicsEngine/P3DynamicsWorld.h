/**
 * I guess inside this world we have physics enabled. Newton must be proud!
 * @author: Quan Bui
 * @version: 04/09/2020
 */

#pragma once

#ifndef P3_DYNAMICS_WORLD_H
#define P3_DYNAMICS_WORLD_H

#include "RigidBody.h"

#include <vector>

// This class can have a container that contains all the physics bodies in the world.
class P3DynamicsWorld
{
public:
	P3DynamicsWorld() {}
	P3DynamicsWorld(size_t maxCapacity) : mMaxCapacity(maxCapacity) {}	// Might want error checking here

	float getGravity() const { return mGravity; }
	float getAirDrag() const { return mAirDrag; }

	void setGravity(float gravity) { mGravity = gravity; }
	void setGravityToBody(DynamicBody &body) { body.setGravity(glm::vec3(0.f, -mGravity, 0.f)); }
	void setMaxCapacity(const int maxCapacity) { mMaxCapacity = maxCapacity; }	// Need error checking

	size_t getMaxCapacity() const { return mMaxCapacity; }

	bool addDynamicBody(glm::vec3 const &position, BoundingVolume *bound, float bounce = 0.f, float mass = 1.f);
	bool addStaticBody(glm::vec3 const &position, BoundingVolume *bound, float bounce = 0.f);
	bool addKinematicBody(glm::vec3 const &position, BoundingVolume *bound, float bounce = 0.f);

	bool isFull() { return mRigidBodyPtrContainer.size() == mMaxCapacity; }

	// Broad phase AABB style. This could be expanded a whole lot, with complicated trees
	bool checkCollision(DynamicBody const &, DynamicBody const &);
	bool checkCollision(DynamicBody const &, StaticBody const &);

	// Should be able to modify the bodies, such as their linear velocity
	void resolveCollision(DynamicBody &, DynamicBody &);
	void resolveCollision(DynamicBody &, StaticBody &);
	void resolveCollision(DynamicBody &, KinematicBody &);

private:
	float mGravity{0.001f}, mAirDrag{2.0f};
	size_t mMaxCapacity{20};
	std::vector<RigidBody *> mRigidBodyPtrContainer;	// How to identify a body in this container??
};

#endif // P3_DYNAMICS_WORLD_H