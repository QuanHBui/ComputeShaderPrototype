#include "Shape.h"

#ifndef NOMINMAX
#define NOMINMAX		// Too many max macro the compiler got confused I guess -Quan Bui
#endif

#include <iostream>
#include <cassert>
#include <limits>

#include "GLSL.h"
#include "Program.h"

#define EPSILON 0.001f

using namespace std;

Shape::~Shape()
{
	glDeleteVertexArrays(1, &vaoID);

	glDeleteBuffers(1, &eleBufID);
	glDeleteBuffers(1, &posBufID);

	if (norBufID) glDeleteBuffers(1, &norBufID);
	if (texBufID) glDeleteBuffers(1, &texBufID);
}

// copy the data from the shape to this object
void Shape::createShape(tinyobj::shape_t const &shape)
{
	posBuf = shape.mesh.positions;
	eleBuf = shape.mesh.indices;
	texBuf = shape.mesh.texcoords;

	// Handle .obj file w/o normals defined
	if (shape.mesh.normals.empty()) {
		normalGen();
	}
	else {
		norBuf = shape.mesh.normals;
	}
}

void Shape::measure()
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = std::numeric_limits<float>::max();
	maxX = maxY = maxZ = -std::numeric_limits<float>::max();

	// Go through all vertices to determine min and max of each dimension
	for (size_t v = 0; v < posBuf.size() / 3; ++v)
	{
		if (posBuf[3*v + 0] < minX) minX = posBuf[3*v + 0];
		if (posBuf[3*v + 0] > maxX) maxX = posBuf[3*v + 0];

		if (posBuf[3*v + 1] < minY) minY = posBuf[3*v + 1];
		if (posBuf[3*v + 1] > maxY) maxY = posBuf[3*v + 1];

		if (posBuf[3*v + 2] < minZ) minZ = posBuf[3*v + 2];
		if (posBuf[3*v + 2] > maxZ) maxZ = posBuf[3*v + 2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
	max.x = maxX;
	max.y = maxY;
	max.z = maxZ;
}

// Stole this from Zoe's defer shading base code
void Shape::resize()
{
	float scaleX, scaleY, scaleZ;
	float shiftX, shiftY, shiftZ;

	// From min and max compute necessary scale and shift for each dimension
	float maxExtent, xExtent, yExtent, zExtent;
	xExtent = max.x - min.x;
	yExtent = max.y - min.y;
	zExtent = max.z - min.z;

	if (xExtent >= yExtent && xExtent >= zExtent)
	{
		maxExtent = xExtent;
	}
	if (yExtent >= xExtent && yExtent >= zExtent)
	{
		maxExtent = yExtent;
	}
	if (zExtent >= xExtent && zExtent >= yExtent)
	{
		maxExtent = zExtent;
	}

	scaleX = 2.0f / maxExtent;
	shiftX = min.x + (xExtent / 2.0f);
	scaleY = 2.0f / maxExtent;
	shiftY = min.y + (yExtent / 2.0f);
	scaleZ = 2.0f / maxExtent;
	shiftZ = min.z + (zExtent / 2.0f);

	// Go through all verticies shift and scale them
	for (size_t v = 0; v < posBuf.size() / 3; ++v)
	{
		posBuf[3*v+0] = (posBuf[3*v+0] - shiftX) * scaleX;
		assert(posBuf[3*v+0] >= -1.0f - EPSILON);
		assert(posBuf[3*v+0] <= 1.0f + EPSILON);
		posBuf[3*v+1] = (posBuf[3*v+1] - shiftY) * scaleY;
		assert(posBuf[3*v+1] >= -1.0f - EPSILON);
		assert(posBuf[3*v+1] <= 1.0f + EPSILON);
		posBuf[3*v+2] = (posBuf[3*v+2] - shiftZ) * scaleZ;
		assert(posBuf[3*v+2] >= -1.0f - EPSILON);
		assert(posBuf[3*v+2] <= 1.0f + EPSILON);
	}
}

void Shape::normalGen()
{
	// Have to make sure that the normal buffer is empty. Maybe not
	if (!norBuf.empty() || posBuf.empty())
		return;

	glm::vec3 v0, v1, v2, normalVec, sumNorm;

	// Initialize normal buffer. Its size should be number of faces x 3 or just the number of vertices
	norBuf.assign(posBuf.size(), 0.f);

	int faceLoopCounter;
	std::vector<unsigned int>::const_iterator faceIter;
	for (faceIter = eleBuf.begin(), faceLoopCounter = 0; faceIter != eleBuf.end(); faceIter += 3u, ++faceLoopCounter) {
		v0 = glm::vec3(	posBuf.at(*(faceIter) * 3u),
						posBuf.at(*(faceIter) * 3u + 1u),
						posBuf.at(*(faceIter) * 3u + 2u) );
		v1 = glm::vec3( posBuf.at(*(faceIter + 1u) * 3u),
						posBuf.at(*(faceIter + 1u) * 3u + 1u),
						posBuf.at(*(faceIter + 1u) * 3u + 2u) );
		v2 = glm::vec3( posBuf.at(*(faceIter + 2u) * 3u),
						posBuf.at(*(faceIter + 2u) * 3u + 1u),
						posBuf.at(*(faceIter + 2u) * 3u + 2u) );

		normalVec = glm::cross(v1 - v0, v2 - v0);

		norBuf.at(*(faceIter) * 3u) += normalVec.x;
		norBuf.at(*(faceIter) * 3u + 1u) += normalVec.y;
		norBuf.at(*(faceIter) * 3u + 2u) += normalVec.z;

		norBuf.at(*(faceIter + 1u) * 3u) += normalVec.x;
		norBuf.at(*(faceIter + 1u) * 3u + 1u) += normalVec.y;
		norBuf.at(*(faceIter + 1u) * 3u + 2u) += normalVec.z;

		norBuf.at(*(faceIter + 2u) * 3u) += normalVec.x;
		norBuf.at(*(faceIter + 2u) * 3u + 1u) += normalVec.y;
		norBuf.at(*(faceIter + 2u) * 3u + 2u) += normalVec.z;
	}

	// Normalize the summation of all normals per vertex
	std::vector<float>::iterator norIter;
	for (norIter = norBuf.begin(); norIter != norBuf.end(); norIter += 3u) {
		sumNorm = glm::vec3(	*(norIter),
								*(norIter + 1u),
								*(norIter + 2u));

		sumNorm = glm::normalize(sumNorm);
		*(norIter) = sumNorm.x;
		*(norIter + 1u) = sumNorm.y;
		*(norIter + 2u) = sumNorm.z;
	}
}

void Shape::init()
{
	// Initialize the vertex array object
	CHECKED_GL_CALL(glGenVertexArrays(1, &vaoID));
	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Send the position array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &posBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW));

	// Send the normal array to the GPU
	if (norBuf.empty())
	{
		norBufID = 0;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW));
	}

	// Send the texture array to the GPU
	if (texBuf.empty())
	{
		texBufID = 0;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &texBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW));
	}

	// Send the element array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &eleBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW));

	// Unbind the arrays
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void Shape::draw(shared_ptr<Program> const &prog, GLuint colorCollisionBufferID)
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);

	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if (h_nor != -1 && norBufID != 0)
	{
		GLSL::enableVertexAttribArray(h_nor);
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
	}

	if (texBufID != 0)
	{
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");

		if (h_tex != -1 && texBufID != 0)
		{
			GLSL::enableVertexAttribArray(h_tex);
			CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
			CHECKED_GL_CALL(glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
		}
	}

	if (colorCollisionBufferID) {
		GLSL::enableVertexAttribArray(2u);
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorCollisionBufferID));
		CHECKED_GL_CALL(glVertexAttribPointer(2u, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0));
	}

	// Bind element buffer
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));

	// Draw
	CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0));

	// Disable and unbind
	if (h_tex != -1)
	{
		GLSL::disableVertexAttribArray(h_tex);
	}
	if (h_nor != -1)
	{
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	CHECKED_GL_CALL(glBindVertexArray(0));
}