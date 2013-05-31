/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#include "Assert.h"
#include "Types.h"
#include "MathUtils.h"

namespace crown
{

/// 4D column vector.
class Vec4
{
public:

	real				x, y, z, w;

	/// Does nothing for efficiency.
						Vec4();	

	/// Initializes all the components to val						
						Vec4(real val);

	/// Constructs from four components								
						Vec4(real nx, real ny, real nz, real nw);

	/// Constructs from array	
						Vec4(const real v[4]);						
						Vec4(const Vec4& a);

	/// Random access by index
	real				operator[](uint32_t i) const;	

	/// Random access by index
	real&				operator[](uint32_t i);						

	Vec4				operator+(const Vec4& a) const;				
	Vec4&				operator+=(const Vec4& a);					
	Vec4 				operator-(const Vec4& a) const;				
	Vec4&				operator-=(const Vec4& a);					
	Vec4				operator*(real k) const;					
	Vec4&				operator*=(real k);							
	Vec4				operator/(real k) const;					
	Vec4&				operator/=(real k);

	/// Dot product							
	real				dot(const Vec4& a) const;					

	/// For simmetry
	friend Vec4			operator*(real k, const Vec4& a);			

	bool				operator==(const Vec4& other) const;		
	bool				operator!=(const Vec4& other) const;

	/// Returns whether all the components of this vector are smaller than all of the @a other vector	
	bool				operator<(const Vec4& other) const;	

	/// Returns whether all the components of this vector are greater than all of the @a other vector		
	bool				operator>(const Vec4& other) const;			

	/// Returns the vector's length
	real				length() const;	

	/// Returns the vector's squared length							
	real				squared_length() const;

	/// Sets the vector's length						
	void				set_length(real len);

	/// Normalizes the vector						
	Vec4&				normalize();

	/// Returns the normalized vector								
	Vec4				get_normalized() const;

	/// Negates the vector (i.e. builds the inverse)						
	Vec4&				negate();	

	/// Negates the vector (i.e. builds the inverse)								
	Vec4				operator-() const;							

	/// Returns the distance
	real				get_distance_to(const Vec4& a) const;

	/// Returns the angle in radians		
	real				get_angle_between(const Vec4& a) const;		

	/// Sets all components to zero
	void				zero();										

	/// Returns the pointer to the vector's data
	real*				to_float_ptr();	

	/// Returns the pointer to the vector's data							
	const real*			to_float_ptr() const;						

	static const Vec4	ZERO;
	static const Vec4	ONE;
	static const Vec4	XAXIS;
	static const Vec4	YAXIS;
	static const Vec4	ZAXIS;
	static const Vec4	WAXIS;
};

//-----------------------------------------------------------------------------
inline Vec4::Vec4()
{
}

//-----------------------------------------------------------------------------
inline Vec4::Vec4(real val) : x(val), y(val), z(val), w(val)
{
}

//-----------------------------------------------------------------------------
inline Vec4::Vec4(real nx, real ny, real nz, real nw) : x(nx), y(ny), z(nz), w(nw)
{
}

//-----------------------------------------------------------------------------
inline Vec4::Vec4(const real a[4]) : x(a[0]), y(a[1]), z(a[2]), w(a[3])
{
}

//-----------------------------------------------------------------------------
inline Vec4::Vec4(const Vec4& a) : x(a.x), y(a.y), z(a.z), w(a.w)
{
}

//-----------------------------------------------------------------------------
inline real Vec4::operator[](uint32_t i) const
{
	CE_ASSERT(i < 4, "Index must be < 4");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline real& Vec4::operator[](uint32_t i)
{
	CE_ASSERT(i < 4, "Index must be < 4");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::operator+(const Vec4& a) const
{
	return Vec4(x + a.x, y + a.y, z + a.z, w + a.w);
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::operator+=(const Vec4& a)
{
	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::operator-(const Vec4& a) const
{
	return Vec4(x - a.x, y - a.y, z - a.z, w - a.w);
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::operator-=(const Vec4& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::operator*(real k) const
{
	return Vec4(x * k, y * k, z * k, w * k);
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::operator*=(real k)
{
	x *= k;
	y *= k;
	z *= k;
	w *= k;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::operator/(real k) const
{
	CE_ASSERT(k != (real)0.0, "Division by zero");

	real inv = (real)(1.0 / k);

	return Vec4(x * inv, y * inv, z * inv, w * inv);
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::operator/=(real k)
{
	CE_ASSERT(k != (real)0.0, "Division by zero");

	real inv = (real)(1.0 / k);

	x *= inv;
	y *= inv;
	z *= inv;
	w *= inv;

	return *this;
}

//-----------------------------------------------------------------------------
inline real Vec4::dot(const Vec4& a) const
{
	return x * a.x + y * a.y + z * a.z + w * a.w;
}

//-----------------------------------------------------------------------------
inline Vec4 operator*(real k, const Vec4& a)
{
	return a * k;
}

//-----------------------------------------------------------------------------
inline bool Vec4::operator==(const Vec4& other) const
{
	return math::equals(x, other.x) && math::equals(y, other.y) && math::equals(z, other.z) && math::equals(w, other.w);
}

//-----------------------------------------------------------------------------
inline bool Vec4::operator!=(const Vec4& other) const
{
	return !math::equals(x, other.x) || !math::equals(y, other.y) || !math::equals(z, other.z) || !math::equals(w, other.w);
}

//-----------------------------------------------------------------------------
inline bool Vec4::operator<(const Vec4& other) const
{
	return ((x < other.x) && (y < other.y) && (z < other.z) && (w < other.w));
}

//-----------------------------------------------------------------------------
inline bool Vec4::operator>(const Vec4& other) const
{
	return ((x > other.x) && (y > other.y) && (z > other.z) && (w > other.w));
}

//-----------------------------------------------------------------------------
inline real Vec4::length() const
{
	return math::sqrt(x * x + y * y + z * z + w * w);
}

//-----------------------------------------------------------------------------
inline real Vec4::squared_length() const
{
	return x * x + y * y + z * z + w * w;
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::normalize()
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
	w *= len;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::get_normalized() const
{
	Vec4 tmp(x, y, z, w);

	return tmp.normalize();
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::negate()
{
	x = -x;
	y = -y;
	z = -z;
	w = -w;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::operator-() const
{
	return Vec4(-x, -y, -z, -w);
}

//-----------------------------------------------------------------------------
inline real Vec4::get_distance_to(const Vec4& a) const
{
	return (*this - a).length();
}

//-----------------------------------------------------------------------------
inline real Vec4::get_angle_between(const Vec4& a) const
{
	return math::acos(this->dot(a) / (this->length() * a.length()));
}

//-----------------------------------------------------------------------------
inline void Vec4::zero()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
	w = 0.0;
}

//-----------------------------------------------------------------------------
inline real* Vec4::to_float_ptr()
{
	return &x;
}

//-----------------------------------------------------------------------------
inline const real* Vec4::to_float_ptr() const
{
	return &x;
}

} // namespace crown

