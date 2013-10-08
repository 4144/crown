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

#include "Quaternion.h"
#include "Vector3.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"


namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int quaternion(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3(1);
	float w = stack.get_float(2);

	stack.push_quaternion(Quaternion(v, w));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int quaternion_negate(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternion(1);

	q.negate();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int quaternion_load_identity(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternion(1);

	q.load_identity();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int quaternion_length(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternion(1);

	stack.push_float(q.length());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int quaternion_conjugate(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternion(1);

	stack.push_quaternion(q.get_conjugate());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int quaternion_inverse(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternion(1);

	stack.push_quaternion(q.get_inverse());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int quaternion_cross(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q1 = stack.get_quaternion(1);
	Quaternion& q2 = stack.get_quaternion(2);

	stack.push_quaternion(q1 * q2);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int quaternion_multiply(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternion(1);
	float k = stack.get_float(2);

	stack.push_quaternion(q * k);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int quaternion_power(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternion(1);
	float k = stack.get_float(2);

	stack.push_quaternion(q.power(k));

	return 1;
}

//-----------------------------------------------------------------------------
void load_quaternion(LuaEnvironment& env)
{
	env.load_module_function("Quaternion", "new",			quaternion);
	env.load_module_function("Quaternion", "negate",		quaternion_negate);
	env.load_module_function("Quaternion", "load_identity",	quaternion_load_identity);
	env.load_module_function("Quaternion", "length",		quaternion_length);
	env.load_module_function("Quaternion", "conjugate",		quaternion_conjugate);
	env.load_module_function("Quaternion", "inverse",		quaternion_inverse);
	env.load_module_function("Quaternion", "cross",			quaternion_cross);
	env.load_module_function("Quaternion", "mul",			quaternion_multiply);
	env.load_module_function("Quaternion", "pow",			quaternion_power);
}

} //namespace crown