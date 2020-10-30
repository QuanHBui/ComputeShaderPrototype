#pragma once

#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <glad/glad.h>
#include <memory>
#include <vector>
#include <glm/mat4x4.hpp>

#include "../PrototypePhysicsEngine/P3Collider.h"

using MatrixContainer = std::vector<glm::mat4>;
using MatrixContainerConstIter = MatrixContainer::const_iterator;

struct CollisionPairGpuPackage;

// Stores all the renderable stuff and OpenGL bookkeepings
class RenderSystem
{
public:
	enum Mesh
	{
		QUAD = 0,
		CUBE,
		SPHERE,
		BOWLING_PIN
	};

	enum ShaderProg
	{
		NORMAL = 0,
		DEBUG
	};

	void init();

	void render(int, int, std::shared_ptr<MatrixContainer>, CollisionPairGpuPackage const &);
	void renderDebug(std::vector<P3BoxCollider> const &);

	void registerMeshForBody(Mesh const &, unsigned int = 1u);
	void setView(glm::mat4 const &view) { mView = view; }
	void setProjection(glm::mat4 const &projection) { mProjection = projection; }

	void reset();

private:
	void initRenderPrograms();
	void initMeshes();
	void initDebug();

	std::vector<std::shared_ptr<class Program>> mpProgramContainer;
	std::vector<std::shared_ptr<class Shape>> mpMeshContainer;
	std::vector<Mesh> mMeshKeyContainer;

	glm::mat4 mView, mProjection;

	//================ For debugging ================//
	GLuint mDebugVao = 0u, mDebugVbo = 0u;

};

#endif // RENDER_SYSTEM_H