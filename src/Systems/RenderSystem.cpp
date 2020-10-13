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
	// Send model matrix

	for (MatrixContainerConstIter it = pModelMatrixContainer->begin();
		it != pModelMatrixContainer->end();
		++it)
	{
		glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "projection"),
			1, GL_FALSE, glm::value_ptr(mProjection));
		glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "view"),
			1, GL_FALSE, glm::value_ptr(mView));
		glUniformMatrix4fv(glGetUniformLocation(mpRenderProgram->getPID(), "model"),
			1, GL_FALSE, glm::value_ptr(*it));
		mpMeshContainer[BOWLING_PIN]->draw(mpRenderProgram);
	}
	
	mpRenderProgram->unbind();
}

void RenderSystem::initRenderPrograms()
{
	GLSL::checkVersion();

	// Set background color
	glClearColor(.12f, .34f, .56f, 1.0f);

	// Enabel z-buffer test
	glEnable(GL_DEPTH_TEST);

	std::shared_ptr<Program> mpRenderProgram = std::make_unique<Program>();
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
