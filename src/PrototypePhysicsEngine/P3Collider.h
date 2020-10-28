#pragma once

#ifndef P3_COLLIDER_H
#define P3_COLLIDER_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

/**
 * Reference: https://blog.winter.dev/2020/gjk-algorithm/
 */

class P3Simplex;

class P3Collider
{
public:
	virtual glm::vec3 findFarthestPoint(glm::vec3 const &) const = 0;
};

class P3MeshCollider : public P3Collider
{
public:
	void update(glm::mat4 const &model)
	{
		for (glm::vec4 &vertex : mVertices)
		{
			vertex = model * vertex;
			vertex.w = 1.0f;
		}
	}

	void setVertices(std::vector<glm::vec4> const &vertices)
	{
		mVertices = vertices;
	}

	glm::vec3 findFarthestPoint(glm::vec3 const &) const override;

private:
	std::vector<glm::vec4> mVertices =
	{
		glm::vec4{ -1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f,  1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f,  1.0f,  1.0f },

		glm::vec4{ -1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f, -1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f, -1.0f,  1.0f }
	};	// A unit box
};

// A collection of 8 vec4's
struct P3BoxCollider
{
	void update(glm::mat4 const &model)
	{
		for (glm::vec4 &vertex : mVertices)
		{
			vertex = model * vertex;
			vertex.w = 1.0f;
		}
	}

	void setVertices(glm::vec4 *vertices)
	{
		for (int i = 0; i < 8; ++i)
		{
			mVertices[i] = vertices[i];
		}
	}

	glm::vec4 mVertices[8] =
	{
		glm::vec4{ -1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f,  1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f,  1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f,  1.0f,  1.0f },

		glm::vec4{ -1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f,  1.0f, -1.0f,  1.0f },
		glm::vec4{  1.0f, -1.0f, -1.0f,  1.0f },
		glm::vec4{ -1.0f, -1.0f, -1.0f,  1.0f }
	};
};

bool P3Gjk(P3Collider const &, P3Collider const &, P3Simplex &);
void P3Epa(P3Collider const &, P3Collider const &, P3Simplex &);

#endif // P3_COLLIDER_H