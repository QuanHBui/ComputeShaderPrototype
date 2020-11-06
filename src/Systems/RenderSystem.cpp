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
		for (glm::vec4 const &collisonPair : collisionPairs.collisionPairs)
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

	// Render the platform
	// glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3{ 0.0f, -4.0f, -20.0f });
	// model *= glm::scale(glm::mat4(1.0f), glm::vec3{ 5.0f, 1.0f, 20.0f });
	// glUniformMatrix4fv(glGetUniformLocation(renderProgram.getPID(), "projection"),
	// 	1, GL_FALSE, glm::value_ptr(mProjection));
	// glUniformMatrix4fv(glGetUniformLocation(renderProgram.getPID(), "view"),
	// 	1, GL_FALSE, glm::value_ptr(mView));
	// glUniformMatrix4fv(glGetUniformLocation(renderProgram.getPID(), "model"),
	// 	1, GL_FALSE, glm::value_ptr(model));
	// mMeshes[QUAD].draw(renderProgram);

	renderProgram.unbind();
}

void RenderSystem::renderDebug(std::vector<P3BoxCollider> const &boxColliders)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(mDebugVao);
	glBindBuffer(GL_ARRAY_BUFFER, mDebugVbo);

	// Bind render program
	Program const &debugShaderProg = mPrograms[ShaderProg::DEBUG];
	debugShaderProg.bind();

	glUniformMatrix4fv(glGetUniformLocation(debugShaderProg.getPID(), "projection"),
		1, GL_FALSE, glm::value_ptr(mProjection));
	glUniformMatrix4fv(glGetUniformLocation(debugShaderProg.getPID(), "view"),
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

	debugShaderProg.unbind();
}

void RenderSystem::registerMeshForBody(MeshKey const &shape, unsigned int quantity)
{
	assert(mNextMeshKeyIdx + 1u < num_mesh_keys && "Mesh keys at limit.");

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

	// Enabel z-buffer test
	glEnable(GL_DEPTH_TEST);

	Program &renderProgram = mPrograms[mNextProgIdx++];
	renderProgram.setVerbose(true);
	renderProgram.setShaderNames(
		"../resources/shaders/vs.vert",
		"../resources/shaders/fs.frag");
	renderProgram.init();
	renderProgram.addAttribute("vertPos");
	renderProgram.addAttribute("vertNor");
	renderProgram.addAttribute("vertTex");
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
	glGenVertexArrays(1u, &mDebugVao);
	glBindVertexArray(mDebugVao);

	glGenBuffers(1u, &mDebugVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mDebugVbo);

	glEnableVertexAttribArray(0u);
	glVertexAttribPointer(0u, 4u, GL_FLOAT, GL_FALSE, 0u, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindVertexArray(0u);

	Program &debugRenderProgram = mPrograms[mNextProgIdx++];
	debugRenderProgram.setVerbose(true);
	debugRenderProgram.setShaderNames(
		"../resources/shaders/debug.vert",
		"../resources/shaders/debug.frag");
	debugRenderProgram.init();
	debugRenderProgram.addAttribute("vertPos");
}
