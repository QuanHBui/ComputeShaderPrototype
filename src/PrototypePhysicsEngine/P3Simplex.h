#pragma once

#ifndef P3_SIMPLEX_H
#define P3_SIMPLEX_H

#include <array>
#include <cmath>
#include <glm/vec3.hpp>

/**
 * A simple std::array wrapper specialized for GJK alogrithm.
 *
 * Reference: https://blog.winter.dev/2020/gjk-algorithm/
 */
class P3Simplex
{
public:
	P3Simplex()
		: mPoints({ glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f) })
		, mSize(0u)
	{}

	P3Simplex& operator=(std::initializer_list<glm::vec3> list)
	{
		for (auto listIter = list.begin(); listIter != list.end(); ++listIter)
			mPoints[std::distance(list.begin(), listIter)] = *listIter;

		mSize = list.size();

		return *this;
	}

	// This is a linked list behavior, but we want to ultilize stack memory for faster access.
	void pushFront(glm::vec3 const& point)
	{
		mPoints = { point, mPoints[0u], mPoints[1u], mPoints[2u] };
		mSize = std::min(mSize + 1u, 4u);
	}

	glm::vec3& operator[](unsigned int i) { return mPoints[i]; }

	unsigned int getSize() const { return mSize; }

	std::array<glm::vec3, 4u>::const_iterator begin() const { return mPoints.begin(); }
	std::array<glm::vec3, 4u>::const_iterator end() const { return mPoints.end(); }

private:
	std::array<glm::vec3, 4u> mPoints;
	unsigned int mSize = 0u;
};

struct SupportPoint
{
	glm::vec3 minkowskiDiffPoint{ 0.0f };	// The position of the Minkowski difference itself.
	glm::vec3 colliderASupport{ 0.0f };
	glm::vec3 colliderBSupport{ 0.0f };

	bool operator==(SupportPoint const& another) const
	{
		return minkowskiDiffPoint == another.minkowskiDiffPoint;
	}

	glm::vec3 operator-(SupportPoint const& another) const
	{
		return minkowskiDiffPoint - another.minkowskiDiffPoint;
	}
};

//================= Data structures to keep track of polytope =================//
// Reference: http://hacktank.net/blog/?p=119
struct TriangleSimplex
{
	SupportPoint points[3];
	glm::vec3 precomputedNormal{ 0.0f };

	TriangleSimplex(SupportPoint const& a, SupportPoint const& b, SupportPoint const& c)
	{
		points[0] = a;
		points[1] = b;
		points[2] = c;

		precomputedNormal = glm::normalize(glm::cross(b - a, c - a));
	}
};

struct EdgeSimplex
{
	SupportPoint points[2];

	EdgeSimplex(SupportPoint const& a, SupportPoint const& b)
	{
		points[0] = a;
		points[1] = b;
	}
};
#endif // P3_SIMPLEX_H