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
	~Shape();

	void createShape(tinyobj::shape_t const &);
	void init();
	void initInstancedBuffers();

	void measure();
	void resize();

	void draw(Program const &, GLuint = 0u);
	void drawInstanced(Program const &, GLsizei, glm::mat4 const *);

	void setMat(tinyobj::material_t const &mat) { mat_ = mat;}

	GLuint getVaoID() const { return vaoID; }
	tinyobj::material_t getMat() const { return mat_; }

	glm::vec3 min = glm::vec3(0);
	glm::vec3 max = glm::vec3(0);

	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;

private:
	void normalGen();
	void enableBasicAttributes(Program const &);
	void disableBasicAttributes();

	GLuint vaoID = 0;

	GLuint eleBufID = 0;
	GLuint posBufID = 0;
	GLuint norBufID = 0;
	GLuint texBufID = 0;

	GLuint instanceMatrices = 0u;

	int h_pos = -1, h_nor = -1, h_tex = -1;

	tinyobj::material_t mat_;
};

#endif // LAB471_SHAPE_H_INCLUDED
