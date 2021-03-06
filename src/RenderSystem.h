#pragma once

#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <glad/glad.h>
#include <glm/mat4x4.hpp>

#include "PrototypePhysicsEngine/P3Collider.h"
#include "Program.h"
#include "Shape.h"

using MatrixContainer          = std::vector<glm::mat4>;
using MatrixContainerConstIter = MatrixContainer::const_iterator;

struct CollisionPairGpuPackage;
struct ManifoldGpuPackage;

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

	void init(int, int);

	void render(MatrixContainer const &, const CollisionPairGpuPackage *, int);
	void renderInstanced(MatrixContainer const &);
	void renderDebug(std::vector<P3BoxCollider> const &, const ManifoldGpuPackage *);

	void registerMeshForBody(MeshKey const &, unsigned int = 1u);
	void setView(glm::mat4 const &view) { mView = view; }
	void setProjection(glm::mat4 const &projection) { mProjection = projection; }

	void reset();

private:
	void initRenderPrograms(int, int);
	void initMeshes();
	void initDebug();

	glm::mat4 mView, mProjection;

	//================ For debugging ================//
	GLuint mDebugVao = 0u, mDebugVbo = 0u;
	//================== Constants ==================//
	static constexpr int shader_count = 3;
	static constexpr int shape_count = 4;
	static constexpr int mesh_key_count = 1024;
	static constexpr int max_mesh_count = 1024;

	Program mPrograms[shader_count];
	Shape   mMeshes[shape_count];
	MeshKey mMeshKeys[mesh_key_count];

	uint8_t mNextShapeIdx   = 0u;
	uint8_t mNextMeshKeyIdx = 0u;
};

#endif // RENDER_SYSTEM_H
