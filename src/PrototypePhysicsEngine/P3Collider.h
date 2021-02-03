#pragma once

#ifndef P3_COLLIDER_H
#define P3_COLLIDER_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

#include "P3Common.h"

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
		for (unsigned int i = 0u; i < mVertices.size(); ++i)
		{
			mVertices[i]   = model * mInstanceVertices[i];
			mVertices[i].w = 1.0f;
		}
	}

	void setInstanceVertices(std::vector<glm::vec4> const &vertices)
	{
		mInstanceVertices = vertices;
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
	}; // A unit box

	std::vector<glm::vec4> mInstanceVertices =
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

// A collection of 8 vec4's
struct P3BoxCollider
{
	void update(glm::mat4 const &model)
	{
		for (int i = 0; i < cBoxColliderVertCount; ++i)
		{
			mVertices[i]   = model * mInstanceVertices[i];
			mVertices[i].w = 1.0f;
		}
	}

	void setInstanceVertices(glm::vec4 *vertices)
	{
		for (int i = 0; i < cBoxColliderVertCount; ++i)
		{
			mInstanceVertices[i] = vertices[i];
		}
	}

	glm::vec4 mVertices[cBoxColliderVertCount] =
	{
		glm::vec4{ -1.0f,  1.0f,  1.0f,  1.0f }, // 0
		glm::vec4{  1.0f,  1.0f,  1.0f,  1.0f }, // 1
		glm::vec4{  1.0f, -1.0f,  1.0f,  1.0f }, // 2
		glm::vec4{ -1.0f, -1.0f,  1.0f,  1.0f }, // 3

		glm::vec4{ -1.0f,  1.0f, -1.0f,  1.0f }, // 4
		glm::vec4{  1.0f,  1.0f, -1.0f,  1.0f }, // 5
		glm::vec4{  1.0f, -1.0f, -1.0f,  1.0f }, // 6
		glm::vec4{ -1.0f, -1.0f, -1.0f,  1.0f }  // 7
	};

	glm::vec4 mInstanceVertices[cBoxColliderVertCount] =
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

#endif // P3_COLLIDER_H
