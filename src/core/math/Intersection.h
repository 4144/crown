/*
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Ray.h"
#include "Plane.h"
#include "Sphere.h"
#include "Frustum.h"
#include "MathUtils.h"
#include "Types.h"
#include "Triangle.h"
#include "Circle.h"
#include "Rect.h"

namespace Crown
{

/**
 * Intersection test utils.
 *
 * Table of Intersection tests (3d)
 * +----------+----------+----------+----------+----------+----------+----------+
 * |          | Ray      | Plane    | Sphere   | Box      | Frustum  | Triangle |
 * +----------+----------+----------+----------+----------+----------+----------+
 * | Ray      | No       | Yes      | Yes      | No       | No       | Yes      |
 * +----------+----------+----------+----------+----------+----------+----------+
 * | Plane    | -        | Yes (1)  | Yes (+)  | No       | No       | No       |
 * +----------+----------+----------+----------+----------+----------+----------+
 * | Sphere   | -        | -        | Yes (+)  | No       | Yes      | Yes (+)  |
 * +----------+----------+----------+----------+----------+----------+----------+
 * | Box      | -        | -        | -        | Yes (+)  | Yes      | No       |
 * +----------+----------+----------+----------+----------+----------+----------+
 * | Frustum  | -        | -        | -        | -        | No       | No       |
 * +----------+----------+----------+----------+----------+----------+----------+
 * | Triangle | -        | -        | -        | -        | -        | No       |
 * +----------+----------+----------+----------+----------+----------+----------+
 *
 * Notes:
 * (1): Intersection of three planes
 * (-): Static intersection only
 * (+): Static/Dynamic intersection
 *
 * Table of Intersection tests (2d)
 * +---------------+----------+-------------+-------------+----------+----------+
 * |               | Circle   | Rect        | O Rect      | Segment  | Ray 2d   |
 * +---------------+----------+-------------+-------------+----------+----------+
 * | Circle        | Yes (p-) | No          | No          | No       | No       |
 * +---------------+----------+-------------+-------------+----------+----------+
 * | Rect          | -        | Yes         | No          | No       | No       | <- Axis Aligned Rect
 * +---------------+----------+-------------+-------------+----------+----------+
 * | O Rect        | -        | -           | No          | No       | No       | <- Oriented Rect
 * +---------------+----------+-------------+-------------+----------+----------+
 * | Segment       | -        | -           | -           | No       | No       |
 * +---------------+----------+-------------+-------------+----------+----------+
 * | Ray 2d        | -        | -           | -           | -        | No       |
 * +---------------+----------+-------------+-------------+----------+----------+
 *
 * Notes:
 * (p): Penetration vector
 * (-): Static intersection only
 * (+): Static/Dynamic intersection
 */
class Intersection
{

public:

	static bool TestRayPlane(const Ray& r, const Plane& p, real& distance, Vec3& intersectionPoint);
	static bool TestRaySphere(const Ray& r, const Sphere& s, real& distance, Vec3& intersectionPoint);
	static bool TestRayBox(const Ray& r, const Box& b, real& distance, Vec3& intersectionPoint);
	static bool TestRayTriangle(const Ray& r, const Triangle& t, real& distance, Vec3& intersectionPoint);

	static bool TestPlane3(const Plane& p1, const Plane& p2, const Plane& p3, Vec3& ip);

	static bool TestStaticSpherePlane(const Sphere& s, const Plane& p);
	static bool TestStaticSphereSphere(const Sphere& a, const Sphere& b);
	static bool TestDynamicSpherePlane(const Sphere& s, const Vec3& d, const Plane& p, real& it, Vec3& intersectionPoint);
	static bool TestDynamicSphereTriangle(const Sphere& s, const Vec3& d, const Triangle& tri, real& it, Vec3& intersectionPoint);
	static bool TestDynamicSphereSphere(const Sphere& s1, const Vec3& d1, const Sphere& s2, const Vec3& d2, real& it, Vec3& intersectionPoint);

	static bool TestStaticBoxBox(const Box& b1, const Box& b2);
	static bool TestDynamicBoxBox(const Box& b1, const Vec3& v1, const Box& b2, const Vec3& v2, real& it);

	static bool TestFrustumSphere(const Frustum& f, const Sphere& s);
	static bool TestFrustumBox(const Frustum& f, const Box& box);

	static bool TestCircleCircle(const Circle& c1, const Circle& c2, Vec2& penetration);
	static bool TestDynamicCircleCircle(const Circle& c1, const Vec2& d1, const Circle& c2, const Vec2& d2, real& it);
	static bool TestRectRect(const Rect& r1, const Rect& r2, Vec2& penetration);
	static bool TestCircleRect(const Circle& c1, const Rect& r2, Vec2& penetration);

private:

	// Disable construction
	Intersection();
};

//-----------------------------------------------------------------------------
inline bool Intersection::TestRayPlane(const Ray& r, const Plane& p, real& distance, Vec3& intersectionPoint)
{
	real nd = r.direction.dot(p.n);
	real orpn = r.origin.dot(p.n);

	if (nd < 0.0)
	{
		real dist = (-p.d - orpn) / nd;
		if (dist > 0.0)
		{
			distance = dist;
			intersectionPoint = r.origin + r.direction * distance;
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestRaySphere(const Ray& r, const Sphere& s, real& distance, Vec3& intersectionPoint)
{
	Vec3 v = s.c - r.origin;
	real b = v.dot(r.direction);
	real det = (s.r * s.r) - v.dot(v) + (b * b);

	if (det < 0.0 || b < s.r)
	{
		return false;
	}

	distance = b - math::sqrt(det);
	intersectionPoint = r.origin + r.direction * distance;

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestRayBox(const Ray& r, const Box& b, real& distance, Vec3& intersectionPoint)
{
	if (r.origin.x < b.min.x)
	{
		if (r.direction.x < 0.0)
		{
			return false;
		}
	}

	if (r.origin.x > b.max.x)
	{
		if (r.direction.x > 0.0)
		{
			return false;
		}
	}

	if (r.origin.y < b.min.y)
	{
		if (r.direction.y < 0.0)
		{
			return false;
		}
	}

	if (r.origin.y > b.max.y)
	{
		if (r.direction.y > 0.0)
		{
			return false;
		}
	}

	if (r.origin.z < b.min.z)
	{
		if (r.direction.z < 0.0)
		{
			return false;
		}
	}

	if (r.origin.z > b.max.z)
	{
		if (r.direction.z > 0.0)
		{
			return false;
		}
	}

	// Possibly intersects
	return true;

	// TODO
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestRayTriangle(const Ray& r, const Triangle& t, real& distance, Vec3& intersectionPoint)
{
	if (Intersection::TestRayPlane(r, t.to_plane(), distance, intersectionPoint))
	{
		if (t.contains_point(intersectionPoint))
		{
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestPlane3(const Plane& p1, const Plane& p2, const Plane& p3, Vec3& ip)
{
	const Vec3& n1 = p1.n;
	const Vec3& n2 = p2.n;
	const Vec3& n3 = p3.n;

	real den = -n1.cross(n2).dot(n3);

	if (math::equals(den, (real)0.0))
	{
		return false;
	}

	Vec3 res = p1.d * n2.cross(n3) + p2.d * n3.cross(n1) + p3.d * n1.cross(n2);
	ip = res / den;

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestStaticSpherePlane(const Sphere& s, const Plane& p)
{
	if (math::abs(p.get_distance_to_point(s.c)) < s.r)
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestStaticSphereSphere(const Sphere& a, const Sphere& b)
{
	real dist = (b.c - a.c).squared_length();
	return (dist < (b.r + a.r) * (b.r + a.r));
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestDynamicSpherePlane(const Sphere& s, const Vec3& d, const Plane& p, real& it, Vec3& intersectionPoint)
{
	const Vec3& sphereCenter = s.c;
	const real sphereRadius = s.r;

	real t0;	// Time at which the sphere intersects the plane remaining at the front side of the plane
	real t1;	// Time at which the sphere intersects the plane remaining at the back side of the plane

	real sphereToPlaneDistance = p.get_distance_to_point(sphereCenter);
	real planeNormalDotVelocity = p.n.dot(d);

	if (planeNormalDotVelocity > 0.0)
	{
		return false;
	}

	// If the sphere is travelling parallel to the plane
	if (planeNormalDotVelocity == 0.0)
	{
		// If the sphere is embedded in the plane
		if (math::abs(sphereToPlaneDistance) < sphereRadius)
		{
			t0 = 0.0;
			t1 = 1.0;

			it = t0;
			intersectionPoint = s.c - p.n * s.r;
			return true;
		}

		return false;
	}

	t0 = (sphereRadius - sphereToPlaneDistance) / planeNormalDotVelocity;
	t1 = (-sphereRadius - sphereToPlaneDistance) / planeNormalDotVelocity;

	// If _both_ t0 and t1 are outside [0,1] then collision can never happen
	if (t0 >= 0.0 && t0 <= 1.0)
	{
		it = math::min(t0, t1);
		intersectionPoint = s.c - p.n * s.r + (d * it);
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestDynamicSphereTriangle(const Sphere& s, const Vec3& d, const Triangle& tri, real& it, Vec3& intersectionPoint)
{
	Plane triPlane = tri.to_plane();

	// Test against the plane containing the triangle
	real spherePlaneIt;
	if (!TestDynamicSpherePlane(s, d, triPlane, spherePlaneIt, intersectionPoint))
	{
		return false;
	}

	// Check if the intersection point lies inside the triangle
	if (tri.contains_point(intersectionPoint))
	{
		it = spherePlaneIt;
		// intersectionPoint is already returned by the above call to TestDynamicSpherePlane
		return true;
	}

	it = spherePlaneIt;

	// Sweep test
	// Check for collision against the vertices
	bool collisionFound = false;
	real a, b, c;
	real x1, x2;

	// v1
	a = d.dot(d);
	b = 2.0 * (d.dot(s.c - tri.v1));
	c = (tri.v1 - s.c).dot(tri.v1 - s.c) - (s.r * s.r);

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		it = math::min(x1, it);
		intersectionPoint = tri.v1;
		collisionFound = true;
	}

	// v2
	b = 2.0 * (d.dot(s.c - tri.v2));
	c = (tri.v2 - s.c).dot(tri.v2 - s.c) - (s.r * s.r);

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		it = math::min(x1, it);
		intersectionPoint = tri.v2;
		collisionFound = true;
	}

	// v3
	b = 2.0 * (d.dot(s.c - tri.v3));
	c = (tri.v3 - s.c).dot(tri.v3 - s.c) - (s.r * s.r);

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		it = math::min(x1, it);
		intersectionPoint = tri.v3;
		collisionFound = true;
	}

	// Check for collisions against the edges
	Vec3 edge;
	Vec3 centerToVertex;
	real edgeDotVelocity;
	real edgeDotCenterToVertex;
	real edgeSquaredLength;
	real velocitySquaredLength;

	velocitySquaredLength = d.squared_length();

	// e1
	edge = tri.v2 - tri.v1;
	centerToVertex = tri.v1 - s.c;
	edgeDotVelocity = edge.dot(d);
	edgeDotCenterToVertex = edge.dot(centerToVertex);
	edgeSquaredLength = edge.squared_length();


	a = edgeSquaredLength * -velocitySquaredLength + edgeDotVelocity * edgeDotVelocity;
	b = edgeSquaredLength * (2.0 * d.dot(centerToVertex)) - (2.0 * edgeDotVelocity * edgeDotCenterToVertex);

	c = edgeSquaredLength * ((s.r * s.r) - centerToVertex.squared_length()) + (edgeDotCenterToVertex * edgeDotCenterToVertex);

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		real f0 = (edgeDotVelocity * x1 - edgeDotCenterToVertex) / edgeSquaredLength;

		if (f0 >= 0.0 && f0 <= 1.0)
		{
			it = math::min(x1, it);
			intersectionPoint = tri.v1 + f0 * edge;
			collisionFound = true;
		}
	}

	// e2
	edge = tri.v3 - tri.v2;
	centerToVertex = tri.v2 - s.c;
	edgeDotVelocity = edge.dot(d);
	edgeDotCenterToVertex = edge.dot(centerToVertex);
	edgeSquaredLength = edge.squared_length();


	a = edgeSquaredLength * -velocitySquaredLength + edgeDotVelocity * edgeDotVelocity;
	b = edgeSquaredLength * (2.0 * d.dot(centerToVertex)) - (2.0 * edgeDotVelocity * edgeDotCenterToVertex);

	c = edgeSquaredLength * ((s.r * s.r) - centerToVertex.squared_length()) + (edgeDotCenterToVertex * edgeDotCenterToVertex);

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		real f0 = (edgeDotVelocity * x1 - edgeDotCenterToVertex) / edgeSquaredLength;

		if (f0 >= 0.0 && f0 <= 1.0)
		{
			it = math::min(x1, it);
			intersectionPoint = tri.v2 + f0 * edge;
			collisionFound = true;
		}
	}

	// e3
	edge = tri.v1 - tri.v3;
	centerToVertex = tri.v3 - s.c;
	edgeDotVelocity = edge.dot(d);
	edgeDotCenterToVertex = edge.dot(centerToVertex);
	edgeSquaredLength = edge.squared_length();


	a = edgeSquaredLength * -velocitySquaredLength + edgeDotVelocity * edgeDotVelocity;
	b = edgeSquaredLength * (2.0 * d.dot(centerToVertex)) - (2.0 * edgeDotVelocity * edgeDotCenterToVertex);

	c = edgeSquaredLength * ((s.r * s.r) - centerToVertex.squared_length()) + (edgeDotCenterToVertex * edgeDotCenterToVertex);

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		real f0 = (edgeDotVelocity * x1 - edgeDotCenterToVertex) / edgeSquaredLength;

		if (f0 >= 0.0 && f0 <= 1.0)
		{
			it = math::min(x1, it);
			intersectionPoint = tri.v3 + f0 * edge;
			collisionFound = true;
		}
	}

	return collisionFound;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestDynamicSphereSphere(const Sphere& s1, const Vec3& d1, const Sphere& s2, const Vec3& d2, real& it, Vec3& intersectionPoint)
{
	// s1 == static sphere
	// s2 == moving sphere
	Vec3 d = d2 - d1;
	d.normalize();

	const Vec3& cs = s1.c;
	const Vec3& cm = s2.c;

	Vec3 e = cs - cm;
	real r = s1.r + s2.r;

	// If ||e|| < r, intersection occurs at t = 0
	if (e.length() < r)
	{
		it = 0.0;
		return true;
	}

	// it == Intersection Time
	real ed = e.dot(d);
	real squared = (ed * ed) + (r * r) - e.dot(e);

	// If the value inside the square root is neg, then no intersection
	if (squared < 0.0)
	{
		return false;
	}

	real t = ed - math::sqrt(squared);
	real l = (d2 - d1).length();

	// If t < 0 || t > l, then non intersection in the considered period of time
	if (t < 0.0 || t > l)
	{
		return false;
	}

	it = t / l;
	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestStaticBoxBox(const Box& b1, const Box& b2)
{
	if (b1.min.x > b2.max.x || b1.max.x < b2.min.x)
	{
		return false;
	}

	if (b1.min.y > b2.max.y || b1.max.y < b2.min.y)
	{
		return false;
	}

	if (b1.min.z > b2.max.z || b1.max.z < b2.min.z)
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestDynamicBoxBox(const Box& b1, const Vec3& v1, const Box& b2, const Vec3& v2, real& it)
{
	// b1 == static box
	// b2 == moving box
	Vec3 d = v2 - v1;

	// Start time of intersection along each axis
	Vec3 tEnterXYZ(0.0, 0.0, 0.0);
	// Stop time of intersection along each axis
	Vec3 tLeaveXYZ(1.0, 1.0, 1.0);

	// If the resulting displacement equals zero, then fallback to static intersection test
	if (math::equals(d.x, (real)0.0))
	{
		if (b1.min.x > b2.max.x || b1.max.x < b2.min.x)
		{
			return false;
		}
	}

	if (math::equals(d.y, (real)0.0))
	{
		if (b1.min.y > b2.max.y || b1.max.y < b2.min.y)
		{
			return false;
		}
	}

	if (math::equals(d.z, (real)0.0))
	{
		if (b1.min.z > b2.max.z || b1.max.z < b2.min.z)
		{
			return false;
		}
	}

	// Otherwise, compute the enter/leave times along each axis
	real oneOverD = (real)(1.0 / d.x);
	tEnterXYZ.x = (b1.min.x - b2.max.x) * oneOverD;
	tLeaveXYZ.x = (b1.max.x - b2.min.x) * oneOverD;

	oneOverD = (real)(1.0 / d.y);
	tEnterXYZ.y = (b1.min.y - b2.max.y) * oneOverD;
	tLeaveXYZ.y = (b1.max.y - b2.min.y) * oneOverD;

	oneOverD = (real)(1.0 / d.z);
	tEnterXYZ.z = (b1.min.z - b2.max.z) * oneOverD;
	tLeaveXYZ.z = (b1.max.z - b2.min.z) * oneOverD;

	// We must ensure that enter time < leave time
	if (tLeaveXYZ.x < tEnterXYZ.x)
	{
		math::swap(tLeaveXYZ.x, tEnterXYZ.x);
	}

	if (tLeaveXYZ.y < tEnterXYZ.y)
	{
		math::swap(tLeaveXYZ.y, tEnterXYZ.y);
	}

	if (tLeaveXYZ.z < tEnterXYZ.z)
	{
		math::swap(tLeaveXYZ.z, tEnterXYZ.z);
	}

	real tEnter = math::max(tEnterXYZ.x, math::max(tEnterXYZ.y, tEnterXYZ.z));
	real tLeave = math::min(tLeaveXYZ.x, math::min(tLeaveXYZ.y, tLeaveXYZ.z));

	// If tEnter > 1, then there is no intersection in the period
	// of time cosidered
	if (tEnter > 1.0)
	{
		return false;
	}

	it = tEnter;

	return tEnter < tLeave;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestFrustumSphere(const Frustum& f, const Sphere& s)
{
	if (f.mPlane[0].get_distance_to_point(s.c) < -s.r || f.mPlane[1].get_distance_to_point(s.c) < -s.r)
	{
		return false;
	}

	if (f.mPlane[2].get_distance_to_point(s.c) < -s.r || f.mPlane[3].get_distance_to_point(s.c) < -s.r)
	{
		return false;
	}

	if (f.mPlane[4].get_distance_to_point(s.c) < -s.r || f.mPlane[5].get_distance_to_point(s.c) < -s.r)
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestFrustumBox(const Frustum& f, const Box& b)
{
	uint vertexOutCount;

	for (uint i = 0; i < 6; i++)
	{
		vertexOutCount = 0;

		for (uint j = 0; j < 8; j++)
		{
			if (f.mPlane[i].get_distance_to_point(b.get_vertex(j)) < 0.0)
			{
				vertexOutCount++;
			}
		}

		// If all vertices are outside one face, then the box doesn't intersect the frustum
		if (vertexOutCount == 8)
		{
			return false;
		}
	}

	// If we are here, is because either the box intersects or it is contained in the frustum
	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestCircleCircle(const Circle& c1, const Circle& c2, Vec2& penetration)
{
	Vec2 distance = c1.c - c2.c;
	real distanceLen2 = distance.squared_length();
	real radiusSum = c1.r + c2.r;
	if (distanceLen2 > radiusSum*radiusSum)
	{
		return false;
	}

	if (distanceLen2 < 0.001)
	{
		penetration = Vec2(c1.r, 0.0);
	}
	else
	{
	  distanceLen2 = math::sqrt(distanceLen2);
		penetration = distance * ((radiusSum - distanceLen2) / distanceLen2);
	}
	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestDynamicCircleCircle(const Circle& c1, const Vec2& d1, const Circle& c2, const Vec2& d2, real& it)
{
	// c1 == static circle
	// c2 == moving circle
	Vec2 d = d2 - d1;
	d.normalize();

	const Vec2& cs = c1.c;
	const Vec2& cm = c2.c;

	Vec2 e = cs - cm;
	real r = c1.r + c2.r;

	// If ||e|| < r, intersection occurs at t = 0
	if (e.length() < r)
	{
		it = 0.0;
		return true;
	}

	// it == Intersection Time
	real ed = e.dot(d);
	real squared = (ed * ed) + (r * r) - e.dot(e);

	// If the value inside the square root is neg, then no intersection
	if (squared < 0.0)
	{
		return false;
	}

	real t = ed - math::sqrt(squared);
	real l = (d2 - d1).length();

	// If t < 0 || t > l, then non intersection in the considered period of time
	if (t < 0.0 || t > l)
	{
		return false;
	}

	it = t / l;
	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestRectRect(const Rect& r1, const Rect& r2, Vec2& penetration)
{
	//x
	real min1MinusMax2 = r1.min.x - r2.max.x;
	real min2MinusMax1 = r2.min.x - r1.max.x;

	if (min1MinusMax2 > min2MinusMax1)
	{
		if (min1MinusMax2 > 0)
		{
			return false;
		}
		penetration.x = -min1MinusMax2;
	}
	else
	{
		if (min2MinusMax1 > 0)
		{
			return false;
		}
		penetration.x = min2MinusMax1;
	}

	//y
	min1MinusMax2 = r1.min.y - r2.max.y;
	min2MinusMax1 = r2.min.y - r1.max.y;

	if (min1MinusMax2 > min2MinusMax1)
	{
		if (min1MinusMax2 > 0)
		{
			return false;
		}
		penetration.y = -min1MinusMax2;
	}
	else
	{
		if (min2MinusMax1 > 0)
		{
			return false;
		}
		penetration.y = min2MinusMax1;
	}

	if (math::abs(penetration.x) < math::abs(penetration.y))
	{
		penetration.y = 0.0;
	}
	else
	{
		penetration.x = 0.0;
	}

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::TestCircleRect(const Circle& c1, const Rect& r2, Vec2& penetration)
{
	bool circleIsAtRight;
	if (c1.c.x > (r2.min.x + r2.max.x) / 2)
	{
		penetration.x = (c1.c.x - c1.r) - r2.max.x;
		circleIsAtRight = true;
	}
	else
	{
		penetration.x = r2.min.x - (c1.c.x + c1.r);
		circleIsAtRight = false;
	}

	bool circleIsAtTop;
	if (c1.c.y > (r2.min.y + r2.max.y) / 2)
	{
		penetration.y = (c1.c.y - c1.r) - r2.max.y;
		circleIsAtTop = true;
	}
	else
	{
		penetration.y = r2.min.y - (c1.c.y + c1.r);
		circleIsAtTop = false;
	}

	if (penetration.x < -c1.r || penetration.y < -c1.r)
	{
		if (penetration.y > 0 || penetration.x > 0)
		{
			return false;
		}
		if (penetration.x > penetration.y)
		{
			penetration.y = 0;
		}
		else
		{
			penetration.x = 0;
		}
	}
	//else if (penetration.y < -c1.r)
	//{
	//	if (penetration.x > 0)
	//	{
	//		return false;
	//	}
	//	penetration.y = 0;
	//}
	else
	{
		penetration += Vec2(c1.r, c1.r);
		real len = math::sqrt(penetration.squared_length());
		if (len > c1.r)
		{
			return false;
		}
		//The - is to point outwards
		penetration *= - (c1.r - len) / len;
	}

	if (circleIsAtRight)
	{
		penetration.x *= -1;
	}

	if (circleIsAtTop)
	{
		penetration.y *= -1;
	}
	
	return true;
}

} // namespace Crown

