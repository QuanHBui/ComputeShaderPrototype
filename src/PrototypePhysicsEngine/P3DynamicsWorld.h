/**
 * I guess inside this world we have physics enabled. Newton must be proud!
 * @author: Quan Bui
 * @version: 02/25/2020
 */

#pragma once

#ifndef P3_DYNAMICS_WORLD_H
#define P3_DYNAMICS_WORLD_H

#include "RigidBody.h"

#include <vector>

// This class can have a container that contains all the physics bodies in the world.
class P3DynamicsWorld
{
private:
	float gravity_{0.001f}, airDrag_{2.f};
	size_t maxCapacity_{20};
	std::vector<RigidBody *> rigidBodyPtrContainer;         // How to identify a body in this container??

public:
	P3DynamicsWorld() {}
	P3DynamicsWorld(size_t maxCapacity) : maxCapacity_{maxCapacity}  {}     // Might want error checking here

	float getGravity() const { return gravity_; }
	float getAirDrag() const { return airDrag_; }

	void setGravity(float gravity) { gravity_ = gravity; }
	void setGravityToBody(DynamicBody &body) { body.setGravity(glm::vec3(0.f, -gravity_, 0.f)); }
	void setMaxCapacity(const int maxCapacity) { maxCapacity_ = maxCapacity; }   // Need error checking

	size_t getMaxCapacity() const { return maxCapacity_; }

	bool addDynamicBody(glm::vec3 const &position, BoundingVolume *bound, float bounce = 0.f, float mass = 1.f);
	bool addStaticBody(glm::vec3 const &position, BoundingVolume *bound, float bounce = 0.f);
	bool addKinematicBody(glm::vec3 const &position, BoundingVolume *bound, float bounce = 0.f);

	bool isFull() { return rigidBodyPtrContainer.size() == maxCapacity_; }

	// Broad phase AABB style. This could be expanded a whole lot, with complicated trees
	bool checkCollision(DynamicBody const &, DynamicBody const &);
	bool checkCollision(DynamicBody const &, StaticBody const &);

	// Should be able to modify the bodies, such as their linear velocity
	void resolveCollision(DynamicBody &, DynamicBody &);
	void resolveCollision(DynamicBody &, StaticBody &);
	void resolveCollision(DynamicBody &, KinematicBody &);
};

#endif // P3_DYNAMICS_WORLD_H