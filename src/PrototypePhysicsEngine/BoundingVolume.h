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
#define MAX_NUM_OBJECTS 1000
#endif

struct BoundingVolume
{
	enum BoundingPrimitives { BOX = 0, SPHERE };

	BoundingVolume() {}
	BoundingVolume(glm::vec3 position) : position_{position} {}
	virtual ~BoundingVolume() {}

	glm::vec3 position_{0.f};
};

// This is not axis-aligned.
struct BoundingBox : public BoundingVolume
{
	BoundingBox() {}
};

struct BoundingSphere : public BoundingVolume
{
	BoundingSphere() {}
	BoundingSphere(float radius) : radius_{radius} {}

	float radius_{0.f};
};

struct PillBox : public BoundingVolume
{

};

struct Cylinder : public BoundingVolume
{

};

struct Aabb
{
	glm::vec4 minBound, maxBound;
};

struct CollisionPair
{
	Aabb firstBody, secondBody;
};

//------------------ Data pack for the GPU (SoA) --------------------//
struct AabbGpuPackage
{
	glm::vec4 minCoords[MAX_NUM_OBJECTS];
	glm::vec4 maxCoords[MAX_NUM_OBJECTS];
};
#endif // BOUNDING_VOLUME_H