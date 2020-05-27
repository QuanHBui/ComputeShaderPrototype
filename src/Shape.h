#pragma once

#ifndef LAB471_SHAPE_H_INCLUDED
#define LAB471_SHAPE_H_INCLUDED

#include <glad/glad.h>
#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader/tiny_obj_loader.h>

class Program;

class Shape
{
public:
	void createShape(tinyobj::shape_t const &);
	void init();
	void measure();
	void resize();
	void draw(std::unique_ptr<Program> const &prog, GLuint colorCollisionBufferID = 0u);

	glm::vec3 min = glm::vec3(0);
	glm::vec3 max = glm::vec3(0);

	void setMat(tinyobj::material_t const &mat) { mat_ = mat;}
	tinyobj::material_t getMat() const { return mat_; }

	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;

	unsigned int eleBufID = 0;
	unsigned int posBufID = 0;
	unsigned int norBufID = 0;
	unsigned int texBufID = 0;
	unsigned int vaoID = 0;

	tinyobj::material_t mat_;

	void normalGen();
};

#endif // LAB471_SHAPE_H_INCLUDED