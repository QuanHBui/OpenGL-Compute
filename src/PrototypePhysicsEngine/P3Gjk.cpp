#include "P3Gjk.h"

#include <glm/glm.hpp>

#include "P3Collider.h"
#include "P3Simplex.h"

#define SAME_DIRECTION(a, b) glm::dot(a, b) > 0.0

bool checkLine(P3Simplex& points, glm::vec3& direction)
{
	SupportPoint supA = points[0];
	SupportPoint supB = points[1];

	glm::vec3 a = supA.mMinkowskiDiffPoint;
	glm::vec3 b = supB.mMinkowskiDiffPoint;

	glm::vec3 ab = b - a;
	glm::vec3 ao = -a;	// From a pointing toward origin

	// If origin is in ao, we keep support point b and a, then 
	//  we choose another support point in the direction that 
	//  spans another dimension, a.k.a a triangle in 2D.
	if (SAME_DIRECTION(ab, ao))
	{
		direction = glm::cross(ao, ab);
	}
	// If not we replace b with another support point in direction of ao
	//  and start again.
	else
	{
		points = { supA };
		direction = ao;
	}

	return false;
}

bool checkTriangle(P3Simplex& points, glm::vec3& direction)
{
	SupportPoint supA = points[0];
	SupportPoint supB = points[1];
	SupportPoint supC = points[2];

	glm::vec3 a = supA.mMinkowskiDiffPoint;
	glm::vec3 b = supB.mMinkowskiDiffPoint;
	glm::vec3 c = supC.mMinkowskiDiffPoint;

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ao = -a;

	glm::vec3 abc = glm::cross(ab, ac);

	// TODO: Need to understand what's going on
	if (SAME_DIRECTION(glm::cross(abc, ac), ao)) {
		if (SAME_DIRECTION(ac, ao)) {
			points = { supA, supC };
			direction = glm::cross(glm::cross(ac, ao), ac);
		}
		else 
		{
			return checkLine(points = { supA, supB }, direction);
		}
	}
	else 
	{
		if (SAME_DIRECTION(glm::cross(ab, abc), ao)) 
		{
			return checkLine(points = { supA, supB }, direction);
		}
		else 
		{
			if (SAME_DIRECTION(abc, ao))
			{
				direction = abc;
			}
			else 
			{
				points = { supA, supC, supB };
				direction = -abc;
			}
		}
	}

	return false;
}

// TODO: Need to understand what's going on
bool checkTetrahedron(P3Simplex& points, glm::vec3& direction)
{
	SupportPoint supA = points[0];
	SupportPoint supB = points[1];
	SupportPoint supC = points[2];
	SupportPoint supD = points[3];

	glm::vec3 a = supA.mMinkowskiDiffPoint;
	glm::vec3 b = supB.mMinkowskiDiffPoint;
	glm::vec3 c = supC.mMinkowskiDiffPoint;
	glm::vec3 d = supD.mMinkowskiDiffPoint;

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ad = d - a;
	glm::vec3 ao = -a;

	glm::vec3 abc = glm::cross(ab, ac);
	glm::vec3 acd = glm::cross(ac, ad);
	glm::vec3 adb = glm::cross(ad, ab);

	if (SAME_DIRECTION(abc, ao))
		return checkTriangle(points = { supA, supB, supC }, direction);

	if (SAME_DIRECTION(acd, ao))
		return checkTriangle(points = { supA, supC, supD }, direction);

	if (SAME_DIRECTION(adb, ao))
		return checkTriangle(points = { supA, supD, supB }, direction);

	return true;
}

bool nextSimplex(P3Simplex& points, glm::vec3& direction)
{
	switch (points.getSize())
	{
	case 2: return checkLine(points, direction);
	case 3: return checkTriangle(points, direction);
	case 4: return checkTetrahedron(points, direction);
	}

	return false;
}

/**
 * Highest level of GJK algorithm
 * 
 * @reference: https://blog.winter.dev/2020/gjk-algorithm/
 */
bool gjk(P3Collider const& colliderA, P3Collider const& colliderB, P3Simplex& points)
{
	// Get inital support point in an arbitrary direction
	SupportPoint supportPoint(colliderA, colliderB, glm::vec3(1.0f, 0.0f, 0.0f));

	// Add support point to the simplex and get the new search direction
	points.pushFront(supportPoint);
	glm::vec3 direction = -supportPoint.mMinkowskiDiffPoint;

	// Find new support point. If this new support point not in front of the search direction,
	//  the loop ends.
	while (true)
	{
		supportPoint = SupportPoint(colliderA, colliderB, direction);

		// Origin not included in the Minkowski difference, so no collision.
		if (glm::dot(supportPoint.mMinkowskiDiffPoint, direction) <= 0.0f)
			return false;

		points.pushFront(supportPoint);

		// Now we have simplex (line -> triangle -> tetrahedral), feed into a function to update 
		//  the simplex and search direction.
		if (nextSimplex(points, direction))
			return true;
	}
}