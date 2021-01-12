/**
 * Could be the base class for all bounding volume shapes
 * TODO: Ideally this class will automatically calculate the current size of the mesh. Can this class take in a Shape object?
 * Also a single mesh might have multiple hitboxes
 * @author: Quan Bui
 * @version: 02/25/2020
 */

#pragma once

#ifndef BOUNDING_VOLUME_H
#define BOUNDING_VOLUME_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#ifndef MAX_NUM_OBJECTS
#define MAX_NUM_OBJECTS 1024
#endif

enum class BoundingType: uint16_t
{
	box = 0,
	sphere,
	capsule,
	convex_hull,
	mesh
};

struct BoundingVolume
{
	BoundingVolume() {}
	BoundingVolume(glm::vec3 position) : mPosition{position} {}
	virtual ~BoundingVolume() {}

	glm::vec3 mPosition{0.f};
};

// This is not axis-aligned.
struct Box : public BoundingVolume
{
	Box() {}
};

struct Sphere : public BoundingVolume
{
	Sphere() {}
	Sphere(float radius) : mRadius{radius} {}

	float mRadius{0.f};
};

struct Capsule : public BoundingVolume
{

};

struct ConvexHull : public BoundingVolume
{

};

struct Mesh : public BoundingVolume
{

};

#endif // BOUNDING_VOLUME_H
