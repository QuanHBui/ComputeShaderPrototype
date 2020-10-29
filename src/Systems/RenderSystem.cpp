#include "RenderSystem.h"

#include <stdexcept>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

void RenderSystem::init()
{
	initRenderPrograms();
	initMeshes();
	initDebug();
}

void RenderSystem::render(int width, int height, std::shared_ptr<MatrixContainer> pModelMatrixContainer)
{
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float aspect = width / (float)height;

	std::shared_ptr<Program> mpRenderProgram = mpProgramContainer[0];

	// Bind render program
	mpRenderProgram->bind();

	std::vector<Mesh>::const_iterator meshContainerIter = mMeshKeyContainer.begin();

	for ( MatrixContainerConstIter it = pModelMatrixContainer->begin()
		; it != pModelMatrixContainer->end()
		; ++it )
	{
		glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "projection"),
			1, GL_FALSE, glm::value_ptr(mProjection));
		glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "view"),
			1, GL_FALSE, glm::value_ptr(mView));
		glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "model"),
			1, GL_FALSE, glm::value_ptr(*it));
		mpMeshContainer[*meshContainerIter]->draw(mpRenderProgram);	// Reusing the loaded meshes

		++meshContainerIter;
		if (meshContainerIter == mMeshKeyContainer.end())
			meshContainerIter = mMeshKeyContainer.begin();
	}

	// Render the platform
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3{ 0.0f, -4.0f, -20.0f });
	model *= glm::scale(glm::mat4(1.0f), glm::vec3{ 5.0f, 1.0f, 20.0f });
	glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "projection"),
		1, GL_FALSE, glm::value_ptr(mProjection));
	glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "view"),
		1, GL_FALSE, glm::value_ptr(mView));
	glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "model"),
		1, GL_FALSE, glm::value_ptr(model));
	mpMeshContainer[QUAD]->draw(mpRenderProgram);

	mpRenderProgram->unbind();
}

void RenderSystem::renderDebug(std::vector<P3BoxCollider> const &boxColliders)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(mDebugVao);
	glBindBuffer(GL_ARRAY_BUFFER, mDebugVbo);

	// Bind render program
	std::shared_ptr<Program> pDebugShaderProg = mpProgramContainer[ShaderProg::DEBUG];
	pDebugShaderProg->bind();

	glUniformMatrix4fv(glGetUniformLocation(pDebugShaderProg->getPID(), "projection"),
		1, GL_FALSE, glm::value_ptr(mProjection));
	glUniformMatrix4fv(glGetUniformLocation(pDebugShaderProg->getPID(), "view"),
		1, GL_FALSE, glm::value_ptr(mView));

	// Iterate through all the box colliders and send the vertices info
	for (P3BoxCollider const &boxCollider : boxColliders)
	{
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec4), (const void *)boxCollider.mVertices, GL_DYNAMIC_DRAW));
		CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLE_FAN, 0, 8));
	}

	glBindVertexArray(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void RenderSystem::registerMeshForBody(Mesh const &shape, unsigned int quantity)
{
	for (unsigned int i = 0u; i < quantity; ++i)
	{
		mMeshKeyContainer.emplace_back(shape);
	}
}

void RenderSystem::reset()
{
	mMeshKeyContainer.clear();
}

void RenderSystem::initRenderPrograms()
{
	GLSL::checkVersion();

	// Set background color
	glClearColor(.12f, .34f, .56f, 1.0f);

	// Enabel z-buffer test
	glEnable(GL_DEPTH_TEST);

	std::shared_ptr<Program> mpRenderProgram = std::make_shared<Program>();
	mpRenderProgram->setVerbose(true);
	mpRenderProgram->setShaderNames(
		"../resources/shaders/vs.vert",
		"../resources/shaders/fs.frag");
	mpRenderProgram->init();
	mpRenderProgram->addAttribute("vertPos");
	mpRenderProgram->addAttribute("vertNor");
	mpRenderProgram->addAttribute("vertTex");

	mpProgramContainer.emplace_back(mpRenderProgram);
}

void RenderSystem::initMeshes()
{
	std::vector<tinyobj::shape_t> TOshapes;
	std::vector<tinyobj::material_t> objMaterials;
	std::string errStr;

	bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/quad.obj");
	if (!rc)
	{
		throw std::runtime_error(errStr);
	}
	else
	{
		std::shared_ptr<Shape> pQuadMesh = std::make_shared<Shape>();
		pQuadMesh->createShape(TOshapes[0]);
		pQuadMesh->measure();
		pQuadMesh->init();

		mpMeshContainer.emplace_back(pQuadMesh);
	}

	rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/cube.obj");
	if (!rc)
	{
		throw std::runtime_error(errStr);
	}
	else
	{
		size_t size = TOshapes.size();

		std::shared_ptr<Shape> pCubeMesh = std::make_shared<Shape>();
		pCubeMesh->createShape(TOshapes[0]);
		pCubeMesh->measure();
		pCubeMesh->init();

		mpMeshContainer.emplace_back(pCubeMesh);
	}

	rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/sphere.obj");
	if (!rc)
	{
		throw std::runtime_error(errStr);
	}
	else
	{
		size_t size = TOshapes.size();

		std::shared_ptr<Shape> pSphereMesh = std::make_shared<Shape>();
		pSphereMesh->createShape(TOshapes[0]);
		pSphereMesh->measure();
		pSphereMesh->resize();
		pSphereMesh->init();

		mpMeshContainer.emplace_back(pSphereMesh);
	}

	rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/bowling_pin.obj");
	if (!rc)
	{
		throw std::runtime_error(errStr);
	}
	else
	{
		size_t size = TOshapes.size();

		std::shared_ptr<Shape> pBowlPinMesh = std::make_shared<Shape>();
		pBowlPinMesh->createShape(TOshapes[0]);
		pBowlPinMesh->measure();
		pBowlPinMesh->resize();
		pBowlPinMesh->init();

		mpMeshContainer.emplace_back(pBowlPinMesh);
	}
}

void RenderSystem::initDebug()
{
	glGenVertexArrays(1, &mDebugVao);
	glBindVertexArray(mDebugVao);

	glGenBuffers(1, &mDebugVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mDebugVbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0u, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindVertexArray(0u);

	std::shared_ptr<Program> pDebugRenderProgram = std::make_shared<Program>();
	pDebugRenderProgram->setVerbose(true);
	pDebugRenderProgram->setShaderNames(
		"../resources/shaders/debug.vert",
		"../resources/shaders/debug.frag");
	pDebugRenderProgram->init();
	pDebugRenderProgram->addAttribute("vertPos");

	mpProgramContainer.emplace_back(pDebugRenderProgram);
}