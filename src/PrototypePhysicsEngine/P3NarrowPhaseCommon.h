#pragma once

#ifndef P3_NARROW_PHASE_COMMON_H
#define P3_NARROW_PHASE_COMMON_H

#include "P3Common.h"

// Heavily based on the definition of Contact from Box2D Lite
struct Contact
{
	glm::vec4 position{};
	glm::vec4 separation{}; // w stores the separation distance
	glm::vec4 referenceRelativePosition{};
	glm::vec4 incidentRelativePosition{};
	glm::vec4 normalTangentBiasImpulses{}; // x = accumulated normal impulse, y = accumulated tangent impulse 1,
										   // z = accumulated tangent impulse 2, w = accumulated normal impulse for position bias
	glm::vec4 normalTangentMassesBias{};   // x = normal mass, y = tangent mass 1, z = tangent mass 2, w = bias factor
};

struct Manifold
{
	glm::ivec4 contactBoxIndicesAndContactCount{}; // x = refBoxIdx, y = incidentBoxIdx, z = contact count
	Contact contacts[cMaxContactPointCount];
	glm::vec4 contactNormal{}; // w stores the penetration depth.
	glm::vec4 contactTangents[2]{};
	glm::vec4 frictionRestitution{};
};

struct ManifoldGpuPackage // To be replaced by the struct below
{
	glm::ivec4 misc{};
	Manifold manifolds[cMaxColliderCount];
};

// Let's make this more data driven, meaning it doesn't make any physical sense but it's easy to move/map data around.
struct ManifoldPackage
{
	glm::ivec4 misc{}; // Must use ivec4 for appropriate memory padding, real data start at offset of 16 bytes
	glm::ivec4 contactBoxIndicesAndContactCount[cMaxColliderCount]{};
	glm::vec4 contactPoints[cMaxContactPointCount][cMaxColliderCount]{};
	glm::vec4 contactNormal[cMaxColliderCount]{};
};

#endif // P3_NARROW_PHASE_COMMON_H
