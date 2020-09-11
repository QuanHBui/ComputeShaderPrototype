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

enum BoundingPrimitives { BOX = 0, SPHERE };

class BoundingVolume
{
public:
	BoundingVolume() {}
	BoundingVolume(glm::vec3 position) : position_{position} {}
	virtual ~BoundingVolume() {}

	glm::vec3 getPosition() const { return position_; }

	void setPosition(glm::vec3 position) { position_ = position; }

protected:
	glm::vec3 position_{0.f};
};

class BoundingBox : public BoundingVolume
{
public:
	BoundingBox() {}
	BoundingBox(glm::vec3 const &minBound, glm::vec3 const &maxBound)
		: minBound_{minBound}, maxBound_{maxBound} {}

	glm::vec3 getMinBound() const { return minBound_; }
	glm::vec3 getMaxBound() const { return maxBound_; }

	void setMinBound(glm::vec3 minBound) { minBound_ = minBound; }
	void setMaxBound(glm::vec3 maxBound) { maxBound_ = maxBound; }

private:
	// minBound should describe lower left -z direction corner,
	//  while maxBound should describe upper right + direction corner. This coordinate system is relative to the object
	glm::vec3 minBound_{0.f}, maxBound_{0.f};
};

class BoundingSphere : public BoundingVolume
{
public:
	BoundingSphere() {}
	BoundingSphere(float radius) : radius_{radius} {}

	float getRadius() const { return radius_; }

	void setRadius(float radius) { radius_ = radius; }

private:
	float radius_{0.f};
};

#endif // BOUNDING_VOLUME_H