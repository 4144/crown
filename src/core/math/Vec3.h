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

#include <cassert>
#include "Types.h"
#include "MathUtils.h"
#include "Vec2.h"

namespace crown
{

/**
	3D column vector.
*/
class Vec3
{
public:

	real				x, y, z;

						Vec3();									//!< Constructor, does nothing for efficiency
						Vec3(real val);							//!< Initializes all the components to val
						Vec3(real nx, real ny, real nz);		//!< Constructs from three components
						Vec3(const real v[3]);					//!< Constructs from array
						Vec3(const Vec3& a);					//!< Copy constructor
						~Vec3();								//!< Destructor

	real				operator[](uint32_t i) const;				//!< Random access by index
	real&				operator[](uint32_t i);						//!< Random access by index

	Vec3				operator+(const Vec3& a) const;			//!< Addition
	Vec3&				operator+=(const Vec3& a);				//!< Addition
	Vec3 				operator-(const Vec3& a) const;			//!< Subtraction
	Vec3&				operator-=(const Vec3& a);				//!< Subtraction
	Vec3				operator*(real k) const;				//!< Multiplication by scalar
	Vec3&				operator*=(real k);						//!< Multiplication by scalar
	Vec3				operator/(real k) const;				//!< Division by scalar
	Vec3&				operator/=(real k);						//!< Division by scalar
	real				dot(const Vec3& a) const;				//!< dot product
	Vec3				cross(const Vec3& a) const;				//!< cross product

	friend Vec3			operator*(real k, const Vec3& a);		//!< For simmetry

	bool				operator==(const Vec3& other) const;	//!< Equality operator
	bool				operator!=(const Vec3& other) const;	//!< Disequality operator
	bool				operator<(const Vec3& other) const;		//!< Returns whether all the components of this vector are smaller than all of the "other" vector
	bool				operator>(const Vec3& other) const;		//!< Returns whether all the components of this vector are greater than all of the "other" vector

	real				length() const;							//!< Returns the vector's length
	real				squared_length() const;					//!< Returns the vector's squared length
	void				set_length(real len);					//!< Sets the vector's length
	Vec3&				normalize();							//!< Normalizes the vector
	Vec3				get_normalized() const;					//!< Returns the normalized vector
	Vec3&				negate();								//!< Negates the vector (i.e. builds the inverse)
	Vec3				operator-() const;						//!< Negates the vector (i.e. builds the inverse)

	real				get_distance_to(const Vec3& a) const;	//!< Returns the distance
	real				get_angle_between(const Vec3& a) const;	//!< Returns the angle in radians

	void				zero();									//!< Builds the zero vector

	real*				to_float_ptr();							//!< Returns the point32_ter to the vector's data
	const real*			to_float_ptr() const;					//!< Returns the point32_ter to the vector's data
	Vec2				to_vec2() const;						//!< Returns a Vec2 with only x and y coordinates

	static const Vec3	ZERO;
	static const Vec3	ONE;
	static const Vec3	XAXIS;
	static const Vec3	YAXIS;
	static const Vec3	ZAXIS;
};

//-----------------------------------------------------------------------------
inline Vec3::Vec3()
{
}

//-----------------------------------------------------------------------------
inline Vec3::Vec3(real val) : x(val), y(val), z(val)
{
}

//-----------------------------------------------------------------------------
inline Vec3::Vec3(real nx, real ny, real nz) : x(nx), y(ny), z(nz)
{
}

//-----------------------------------------------------------------------------
inline Vec3::Vec3(const real v[3]) : x(v[0]), y(v[1]), z(v[2])
{
}

//-----------------------------------------------------------------------------
inline Vec3::Vec3(const Vec3& a) : x(a.x), y(a.y), z(a.z)
{
}

//-----------------------------------------------------------------------------
inline Vec3::~Vec3()
{
}

//-----------------------------------------------------------------------------
inline real Vec3::operator[](uint32_t i) const
{
	assert(i < 3);

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline real& Vec3::operator[](uint32_t i)
{
	assert(i < 3);

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::operator+(const Vec3& a) const
{
	return Vec3(x + a.x, y + a.y, z + a.z);
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::operator+=(const Vec3& a)
{
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::operator-(const Vec3& a) const
{
	return Vec3(x - a.x, y - a.y, z - a.z);
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::operator-=(const Vec3& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::operator*(real k) const
{
	return Vec3(x * k, y * k, z * k);
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::operator*=(real k)
{
	x *= k;
	y *= k;
	z *= k;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::operator/(real k) const
{
	assert(k != (real)0.0);

	real inv = (real)(1.0 / k);

	return Vec3(x * inv, y * inv, z * inv);
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::operator/=(real k)
{
	assert(k != (real)0.0);

	real inv = (real)(1.0 / k);

	x *= inv;
	y *= inv;
	z *= inv;

	return *this;
}

//-----------------------------------------------------------------------------
inline real Vec3::dot(const Vec3& a) const
{
	return x * a.x + y * a.y + z * a.z;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::cross(const Vec3& a) const
{
	return Vec3(y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x);
}

//-----------------------------------------------------------------------------
inline Vec3 operator*(real k, const Vec3& a)
{
	return a * k;
}

//-----------------------------------------------------------------------------
inline bool Vec3::operator==(const Vec3& other) const
{
	return math::equals(x, other.x) && math::equals(y, other.y) && math::equals(z, other.z);
}

//-----------------------------------------------------------------------------
inline bool Vec3::operator!=(const Vec3& other) const
{
	return !math::equals(x, other.x) || !math::equals(y, other.y) || !math::equals(z, other.z);
}

//-----------------------------------------------------------------------------
inline bool Vec3::operator<(const Vec3& other) const
{
	return ((x < other.x) && (y < other.y) && (z < other.z));
}

//-----------------------------------------------------------------------------
inline bool Vec3::operator>(const Vec3& other) const
{
	return ((x > other.x) && (y > other.y) && (z > other.z));
}

//-----------------------------------------------------------------------------
inline real Vec3::length() const
{
	return math::sqrt(x * x + y * y + z * z);
}

//-----------------------------------------------------------------------------
inline real Vec3::squared_length() const
{
	return x * x + y * y + z * z;
}

//-----------------------------------------------------------------------------
inline void Vec3::set_length(real len)
{
	normalize();

	x *= len;
	y *= len;
	z *= len;
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::normalize()
{
	real len = length();

	if (math::equals(len, (real)0.0))
	{
		return *this;
	}

	len = (real)(1.0 / len); 

	x *= len;
	y *= len;
	z *= len;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::get_normalized() const
{
	Vec3 tmp(x, y, z);

	return tmp.normalize();
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::negate()
{
	x = -x;
	y = -y;
	z = -z;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}

//-----------------------------------------------------------------------------
inline real Vec3::get_distance_to(const Vec3& a) const
{
	return (*this - a).length();
}

//-----------------------------------------------------------------------------
inline real Vec3::get_angle_between(const Vec3& a) const
{
	return math::acos(this->dot(a) / (this->length() * a.length()));
}

//-----------------------------------------------------------------------------
inline void Vec3::zero()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
}

//-----------------------------------------------------------------------------
inline real* Vec3::to_float_ptr()
{
	return &x;
}

//-----------------------------------------------------------------------------
inline const real* Vec3::to_float_ptr() const
{
	return &x;
}

//-----------------------------------------------------------------------------
inline Vec2 Vec3::to_vec2() const
{
	return Vec2(x, y);
}

//-----------------------------------------------------------------------------
//!< Returns the parallel portion of "v" projected onto "n"
inline Vec3 get_projected_parallel(const Vec3& v, const Vec3& n)
{
	real n_len_q;
	n_len_q = n.length();
	n_len_q = n_len_q * n_len_q;

	return n * (v.dot(n) / n_len_q);
}

//-----------------------------------------------------------------------------
//!< Returns the perpendicular portion of "v" projected onto "n"
inline Vec3 get_projected_perpendicular(const Vec3& v, const Vec3& n)
{
	return v - get_projected_parallel(v, n);
}

} // namespace crown

