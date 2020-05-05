/**
 * Header file for some rigid bodies type
 * Here we go
 * @author: Quan Bui
 * @version: 02/25/2020
 */

#pragma once

#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <glm/glm.hpp>
#include "BoundingVolume.h"

class RigidBody
{
protected:
	char *name_{nullptr};
	glm::vec3 position_{0.f};
	BoundingVolume *boundPtr_{nullptr};
	float bounce_{0.f};

public:
	RigidBody() {}

	RigidBody(glm::vec3 const &position, BoundingBox *boundBoxPtr, float bounce = 0.f)
		: position_{position}, boundPtr_{boundBoxPtr}, bounce_{bounce} {}

	RigidBody(glm::vec3 const &position, BoundingSphere *boundSpherePtr, float bounce = 0.f)
		: position_{position}, boundPtr_{boundSpherePtr}, bounce_{bounce} {}

	virtual ~RigidBody() {};      // virtual for better clean up and no instance of this class

	glm::vec3 getPosition() const { return position_; }
	BoundingVolume *getBoundPtr() const { return boundPtr_; }

	void setPosition(glm::vec3 const &position) { position_ = position; }
	void setBounce(float bounce) { bounce_ = bounce; }
};

// Bodies that affected by gravity and can move
class DynamicBody : public RigidBody
{
protected:
	glm::vec3 linearVelocity_{0.f}, angularVelocity_{0.f};
	glm::vec3 linearAcceleration_{0.f}, angularAcceleration_{0.f};
	float mass_{0.f};

public:
	DynamicBody() {}

	DynamicBody(glm::vec3 const &position, BoundingBox *boundBoxPtr,
		float bounce = 0.f, float mass = 1.f)
		: RigidBody{ position, boundBoxPtr, bounce }, mass_{mass} {}

	DynamicBody(glm::vec3 const &position, BoundingSphere *boundSpherePtr,
		float bounce = 0.f, float mass = 1.f)
		: RigidBody{ position, boundSpherePtr, bounce }, mass_{mass} {}

	DynamicBody(glm::vec3 const &position, glm::vec3 const &linearVelocity,
		glm::vec3 const &angularVelocity, BoundingBox *boundBoxPtr,
		float bounce = 0.f, float mass = 1.f)
		: RigidBody{ position, boundBoxPtr, bounce }, linearVelocity_{linearVelocity},
		angularVelocity_{angularVelocity}, mass_{mass} {}

	DynamicBody(glm::vec3 const &position, glm::vec3 const &linearVelocity,
		glm::vec3 const &angularVelocity, BoundingSphere *boundSpherePtr,
		float bounce = 0.f, float mass = 1.f)
		: RigidBody{ position, boundSpherePtr, bounce }, linearVelocity_{linearVelocity},
		angularVelocity_{angularVelocity}, mass_{mass} {}

	~DynamicBody() {}

	float getMass() const { return mass_; }
	glm::vec3 getLinearVelocity() const { return linearVelocity_; }
	glm::vec3 getAngularVelocity() const { return angularVelocity_; }

	void setMass(float mass) { mass_ = mass; }
	void setGravity(glm::vec3 const &gravity) { linearAcceleration_ += gravity; }
	void setLinearVelocity(glm::vec3 const &linearVelocity) { linearVelocity_ = linearVelocity; }
	void setAngularVelocity(glm::vec3 const &angularVelocity) { angularVelocity_ = angularVelocity; }

	// Natural move behavior, if given an initial velocity, if not should naturally fall down
	//  This function should be called every rendered frame.
	virtual void move();
	void rotate();
};

// Physics engine will handle the collision detection but the resolution or how the object moves is
//  controlled by the user rather than the physics engine.
class KinematicBody : public DynamicBody
{
public:
	KinematicBody() {}
	// Does player need mass or bounce?

	KinematicBody(glm::vec3 const &position, BoundingBox *boundBoxPtr,
	float bounce = 0.f, float mass = 0.f)
		: DynamicBody{ position, boundBoxPtr, bounce, mass } {}

	KinematicBody(glm::vec3 const &position, BoundingSphere *boundSpherePtr,
	float bounce = 0.f, float mass = 0.f)
		: DynamicBody{ position, boundSpherePtr, bounce, mass } {}

	~KinematicBody() {}

	// Overriding move function. Controlled by player/user
	void move();
};

// Bodies that unaffected by gravity and cannot move.
//  They have zero mass and zero velocity, so any collision between a DynamicBody and a StaticBody will be
//  pretty much elastic collision.
class StaticBody : public RigidBody
{
public:
	StaticBody() {}

	// I suppose a static body can have bounce value
	StaticBody(glm::vec3 const &position, BoundingBox *boundBoxPtr, float bounce = 0.f)
		: RigidBody{ position, boundBoxPtr, bounce } {}

	StaticBody(glm::vec3 const &position, BoundingSphere *boundSpherePtr, float bounce = 0.f)
		: RigidBody{ position, boundSpherePtr, bounce } {}

	~StaticBody() {}
};
#endif // RIGID_BODY_H