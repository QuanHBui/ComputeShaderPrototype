#pragma once

#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <memory>
#include <vector>
#include <glm/mat4x4.hpp>

using MatrixContainer = std::vector<glm::mat4>;
using MatrixContainerConstIter = MatrixContainer::const_iterator;

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

	void init();

	void render(int, int, std::shared_ptr<MatrixContainer>);

	void registerMeshForBody(Mesh const&, unsigned int = 1u);
	void setView(glm::mat4 const& view) { mView = view; }
	void setProjection(glm::mat4 const& projection) { mProjection = projection; }

	void reset();

private:
	void initRenderPrograms();
	void initMeshes();

	std::vector<std::shared_ptr<class Program>> mpProgramContainer;
	std::vector<std::shared_ptr<class Shape>> mpMeshContainer;
	std::vector<Mesh> mMeshKeyContainer;

	glm::mat4 mView, mProjection;
};

#endif // RENDER_SYSTEM_H 