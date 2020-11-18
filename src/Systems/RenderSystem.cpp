#include "RenderSystem.h"

#include <iostream>

#include "../PrototypePhysicsEngine/BoundingVolume.h"
#include "GLSL.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

void RenderSystem::init()
{
	initRenderPrograms();
	initMeshes();
	initDebug();
}

void RenderSystem::render(int width, int height,
	MatrixContainer const &modelMatrices, CollisionPairGpuPackage const &collisionPairs)
{
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float aspect = width / float(height);

	Program const &renderProgram = mPrograms[0];

	// Bind render program
	renderProgram.bind();

	uint8_t objectIdx  = 0u;
	uint8_t meshKeyIdx = 0u;
	MeshKey shapeIdx   = QUAD;

	// Iterate through the input composite model matrices
	for ( MatrixContainerConstIter it = modelMatrices.begin()
		; it != modelMatrices.end()
		; ++it )
	{
		unsigned int redOrNo = 0u;
		// Check collision pair list if this mesh has collided
		for (glm::ivec4 const &collisonPair : collisionPairs.collisionPairs)
		{
			if (collisonPair.x < 0.0) break;

			if (collisonPair.x == objectIdx || collisonPair.y == objectIdx)
			{
				redOrNo = 1u;

				break;
			}
		}

		glUniform1ui(glGetUniformLocation(renderProgram.getPID(), "redOrNo"), redOrNo);
		glUniformMatrix4fv(glGetUniformLocation(renderProgram.getPID(), "projection"),
			1, GL_FALSE, glm::value_ptr(mProjection));
		glUniformMatrix4fv(glGetUniformLocation(renderProgram.getPID(), "view"),
			1, GL_FALSE, glm::value_ptr(mView));
		glUniformMatrix4fv(glGetUniformLocation(renderProgram.getPID(), "model"),
			1, GL_FALSE, glm::value_ptr(*it));

		shapeIdx = mMeshKeys[meshKeyIdx++];

		mMeshes[shapeIdx].draw(renderProgram); // Reusing the loaded meshes

		++objectIdx;
	}

	renderProgram.unbind();
}

void RenderSystem::renderInstanced(int width, int height,
	MatrixContainer const &modelMatrices, CollisionPairGpuPackage const &collisionPairs)
{
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float aspect = width / float(height);

	Program const &renderProgram = mPrograms[NORMAL_INSTANCED];

	// Bind render program
	renderProgram.bind();

	glUniform1ui(glGetUniformLocation(renderProgram.getPID(), "redOrNo"), 0);
	glUniformMatrix4fv(glGetUniformLocation(renderProgram.getPID(), "projection"),
		1, GL_FALSE, glm::value_ptr(mProjection));
	glUniformMatrix4fv(glGetUniformLocation(renderProgram.getPID(), "view"),
		1, GL_FALSE, glm::value_ptr(mView));

	mMeshes[CUBE].drawInstanced(renderProgram, modelMatrices.size() , modelMatrices.data());

	renderProgram.unbind();
}

void RenderSystem::renderDebug(std::vector<P3BoxCollider> const &boxColliders)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(mDebugVao);
	glBindBuffer(GL_ARRAY_BUFFER, mDebugVbo);

	// Bind render program
	Program const &debugShaderProg = mPrograms[DEBUG];
	debugShaderProg.bind();

	glUniformMatrix4fv(glGetUniformLocation(debugShaderProg.getPID(), "projection"),
		1, GL_FALSE, glm::value_ptr(mProjection));
	glUniformMatrix4fv(glGetUniformLocation(debugShaderProg.getPID(), "view"),
		1, GL_FALSE, glm::value_ptr(mView));

	// Iterate through all the box colliders and batch all the vertices
	glm::vec4 batchedVertices[8 * max_mesh_count];
	int colliderIdx = 0;

	for (P3BoxCollider const &boxCollider : boxColliders)
	{
		for (int vertIdx = 0; vertIdx < 8; ++vertIdx)
		{
			batchedVertices[8 * colliderIdx + vertIdx] = boxCollider.mVertices[vertIdx];
		}

		++colliderIdx;
	}

	// Batch draw call
	glBufferData(GL_ARRAY_BUFFER, 8 * boxColliders.size() * sizeof(glm::vec4), (const void *)batchedVertices, GL_DYNAMIC_DRAW);
	CHECKED_GL_CALL(glDrawArrays(GL_POINTS, 0, 8 * boxColliders.size()));

	glBindVertexArray(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	debugShaderProg.unbind();
}

void RenderSystem::registerMeshForBody(MeshKey const &shape, unsigned int quantity)
{
	assert(mNextMeshKeyIdx + 1u < mesh_key_count && "Mesh keys at limit.");

	for (unsigned int i = 0u; i < quantity; ++i)
		mMeshKeys[mNextMeshKeyIdx++] = shape;
}

void RenderSystem::reset()
{
	mNextMeshKeyIdx = 0u;
}

void RenderSystem::initRenderPrograms()
{
	GLSL::checkVersion();

	// Set background color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Program &renderProgram = mPrograms[NORMAL];
	renderProgram.setVerbose(true);
	renderProgram.setShaderNames(
		"../resources/shaders/vs.vert",
		"../resources/shaders/fs.frag");
	renderProgram.init();
	renderProgram.addAttribute("vertPos");
	renderProgram.addAttribute("vertNor");
	renderProgram.addAttribute("vertTex");

	Program &instanceProgram = mPrograms[NORMAL_INSTANCED];
	instanceProgram.setVerbose(true);
	instanceProgram.setShaderNames(
		"../resources/shaders/vs_instanced.vert",
		"../resources/shaders/fs.frag");
	instanceProgram.init();
	instanceProgram.addAttribute("vertPos");
	instanceProgram.addAttribute("vertNor");
	instanceProgram.addAttribute("vertTex");
}

void RenderSystem::initMeshes()
{
	std::vector<tinyobj::shape_t> TOshapes;
	std::vector<tinyobj::material_t> objMaterials;
	std::string errStr;

	bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/quad.obj");
	if (!rc)
	{
		std::cerr << errStr;
	}
	else
	{
		Shape &quadMesh = mMeshes[mNextShapeIdx++];
		quadMesh.createShape(TOshapes[0u]);
		quadMesh.measure();
		quadMesh.init();
	}

	rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/cube.obj");
	if (!rc)
	{
		std::cerr << errStr;
	}
	else
	{
		Shape &cubeMesh = mMeshes[mNextShapeIdx++];
		cubeMesh.createShape(TOshapes[0u]);
		cubeMesh.measure();
		cubeMesh.resize();
		cubeMesh.init();
		cubeMesh.initInstancedBuffers();
	}

	rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/sphere.obj");
	if (!rc)
	{
		std::cerr << errStr;
	}
	else
	{
		Shape &sphereMesh = mMeshes[mNextShapeIdx++];
		sphereMesh.createShape(TOshapes[0u]);
		sphereMesh.measure();
		sphereMesh.resize();
		sphereMesh.init();
	}

	rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/bowling_pin.obj");
	if (!rc)
	{
		std::cerr << errStr;
	}
	else
	{
		Shape &bowlPinMesh = mMeshes[mNextShapeIdx++];
		bowlPinMesh.createShape(TOshapes[0u]);
		bowlPinMesh.measure();
		bowlPinMesh.resize();
		bowlPinMesh.init();
	}
}

void RenderSystem::initDebug()
{
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	glGenVertexArrays(1u, &mDebugVao);
	glBindVertexArray(mDebugVao);

	glGenBuffers(1u, &mDebugVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mDebugVbo);

	glEnableVertexAttribArray(0u);
	glVertexAttribPointer(0u, 4u, GL_FLOAT, GL_FALSE, 0u, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindVertexArray(0u);

	Program &debugRenderProgram = mPrograms[DEBUG];
	debugRenderProgram.setVerbose(true);
	debugRenderProgram.setShaderNames(
		"../resources/shaders/debug.vert",
		"../resources/shaders/debug.frag");
	debugRenderProgram.init();
	debugRenderProgram.addAttribute("vertPos");
}
