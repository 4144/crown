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

#include "vector3.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int vector3_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(Vector3(stack.get_float(1), stack.get_float(2), stack.get_float(3)));
	return 1;
}

static int vector3_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return vector3_new(L);
}

static int vector3_x(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).x);
	return 1;
}

static int vector3_y(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).y);
	return 1;
}

static int vector3_z(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).z);
	return 1;
}

static int vector3_set_x(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).x = stack.get_float(2);
	return 0;
}

static int vector3_set_y(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).y = stack.get_float(2);
	return 0;
}

static int vector3_set_z(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).z = stack.get_float(2);
	return 0;
}

static int vector3_values(lua_State* L)
{
	LuaStack stack(L);
	Vector3& a = stack.get_vector3(1);
	stack.push_float(a.x);
	stack.push_float(a.y);
	stack.push_float(a.z);
	return 3;
}

static int vector3_add(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) + stack.get_vector3(2));
	return 1;
}

static int vector3_subtract(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) - stack.get_vector3(2));
	return 1;
}

static int vector3_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) * stack.get_float(2));
	return 1;
}

static int vector3_divide(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) / stack.get_float(2));
	return 1;
}

static int vector3_dot(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(vector3::dot(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_cross(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(vector3::cross(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_equal(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_vector3(1) == stack.get_vector3(2));
	return 1;
}

static int vector3_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(vector3::length(stack.get_vector3(1)));
	return 1;
}

static int vector3_squared_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(vector3::squared_length(stack.get_vector3(1)));
	return 1;
}

static int vector3_set_length(lua_State* L)
{
	LuaStack stack(L);
	vector3::set_length(stack.get_vector3(1), stack.get_float(2));
	return 0;
}

static int vector3_normalize(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(vector3::normalize(stack.get_vector3(1)));
	return 1;
}

static int vector3_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(vector3::distance(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(vector3::angle(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector2_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(Vector2(stack.get_float(1), stack.get_float(2)));
	return 1;
}

static int vector2_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return vector2_new(L);
}

void load_vector3(LuaEnvironment& env)
{
	env.load_module_function("Vector3", "new",            vector3_new);
	env.load_module_function("Vector3", "x",              vector3_x);
	env.load_module_function("Vector3", "y",              vector3_y);
	env.load_module_function("Vector3", "z",              vector3_z);
	env.load_module_function("Vector3", "set_x",          vector3_set_x);
	env.load_module_function("Vector3", "set_y",          vector3_set_y);
	env.load_module_function("Vector3", "set_z",          vector3_set_z);
	env.load_module_function("Vector3", "values",         vector3_values);
	env.load_module_function("Vector3", "add",            vector3_add);
	env.load_module_function("Vector3", "subtract",       vector3_subtract);
	env.load_module_function("Vector3", "multiply",       vector3_multiply);
	env.load_module_function("Vector3", "divide",         vector3_divide);
	env.load_module_function("Vector3", "dot",            vector3_dot);
	env.load_module_function("Vector3", "cross",          vector3_cross);
	env.load_module_function("Vector3", "equal",          vector3_equal);
	env.load_module_function("Vector3", "length",         vector3_length);
	env.load_module_function("Vector3", "squared_length", vector3_squared_length);
	env.load_module_function("Vector3", "set_length",     vector3_set_length);
	env.load_module_function("Vector3", "normalize",      vector3_normalize);
	env.load_module_function("Vector3", "distance",       vector3_distance);
	env.load_module_function("Vector3", "angle",          vector3_angle);

	env.load_module_constructor("Vector3", vector3_ctor);

	env.load_module_function("Vector2", "new",            vector2_new);
	env.load_module_constructor("Vector2", vector2_ctor);
}

} // namespace crown
