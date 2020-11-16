#pragma once

#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <glad/glad.h>
#include <glm/mat4x4.hpp>

#include "../PrototypePhysicsEngine/P3Collider.h"
#include "Shape.h"
#include "Program.h"

using MatrixContainer          = std::vector<glm::mat4>;
using MatrixContainerConstIter = MatrixContainer::const_iterator;

struct CollisionPairGpuPackage;

// Stores all the renderable stuff and OpenGL bookkeepings
class RenderSystem
{
public:
	enum MeshKey: uint8_t
	{
		QUAD = 0,
		CUBE,
		SPHERE,
		BOWLING_PIN
	};

	enum ShaderProg: uint8_t
	{
		NORMAL = 0,
		DEBUG,
		NORMAL_INSTANCED
	};

	void init();

	void render(int, int, MatrixContainer const &, CollisionPairGpuPackage const &);
	void renderInstanced(int, int, MatrixContainer const &, CollisionPairGpuPackage const &);
	void renderDebug(std::vector<P3BoxCollider> const &);

	void registerMeshForBody(MeshKey const &, unsigned int = 1u);
	void setView(glm::mat4 const &view) { mView = view; }
	void setProjection(glm::mat4 const &projection) { mProjection = projection; }

	void reset();

private:
	void initRenderPrograms();
	void initMeshes();
	void initDebug();

	glm::mat4 mView, mProjection;

	//================ For debugging ================//
	GLuint mDebugVao = 0u, mDebugVbo = 0u;
	//================== Constants ==================//
	static constexpr uint8_t num_shaders   = 3u;
	static constexpr uint8_t num_shapes    = 4u;
	static constexpr uint8_t num_mesh_keys = 100u;

	Program mPrograms[num_shaders];
	Shape   mMeshes[num_shapes];
	MeshKey mMeshKeys[num_mesh_keys];

	uint8_t mNextShapeIdx   = 0u;
	uint8_t mNextMeshKeyIdx = 0u;
};

#endif // RENDER_SYSTEM_H
