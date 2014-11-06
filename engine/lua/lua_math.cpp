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

#include "math_utils.h"
#include "math_types.h"
#include "aabb.h"
#include "intersection.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int math_to_rad(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(to_rad(stack.get_float(1)));
	return 1;
}

static int math_to_deg(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(to_deg(stack.get_float(1)));
	return 1;
}

static int math_next_pow_2(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(next_pow_2(stack.get_int(1)));
	return 1;
}

static int math_is_pow_2(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(is_pow_2(stack.get_int(1)));
	return 1;
}

static int math_ceil(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(ceil(stack.get_float(1)));
	return 1;
}

static int math_floor(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(floor(stack.get_float(1)));
	return 1;
}

static int math_sqrt(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(sqrt(stack.get_float(1)));
	return 1;
}

static int math_inv_sqrt(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(inv_sqrt(stack.get_float(1)));
	return 1;
}

static int math_sin(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(sin(stack.get_float(1)));
	return 1;
}

static int math_cos(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(cos(stack.get_float(1)));
	return 1;
}

static int math_asin(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(asin(stack.get_float(1)));
	return 1;
}

static int math_acos(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(acos(stack.get_float(1)));
	return 1;
}

static int math_tan(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(tan(stack.get_float(1)));
	return 1;
}

static int math_atan2(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(atan2(stack.get_float(1), stack.get_float(2)));
	return 1;
}

static int math_abs(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(abs(stack.get_float(1)));
	return 1;
}

static int math_fmod(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(fmod(stack.get_float(1), stack.get_float(2)));
	return 1;
}

static int math_ray_oobb_intersection(lua_State* L)
{
	LuaStack stack(L);
	OBB oobb;
	oobb.tm = stack.get_matrix4x4(3);
	oobb.aabb.min = stack.get_vector3(4) * -0.5;
	oobb.aabb.max = stack.get_vector3(4) * 0.5;
	stack.push_float(ray_oobb_intersection(stack.get_vector3(1), stack.get_vector3(2), oobb));
	return 1;
}

void load_math(LuaEnvironment& env)
{
	env.load_module_function("Math", "to_rad",                math_to_rad);
	env.load_module_function("Math", "to_deg",                math_to_deg);
	env.load_module_function("Math", "next_pow_2",            math_next_pow_2);
	env.load_module_function("Math", "is_pow_2",              math_is_pow_2);
	env.load_module_function("Math", "ceil",                  math_ceil);
	env.load_module_function("Math", "floor",                 math_floor);
	env.load_module_function("Math", "sqrt",                  math_sqrt);
	env.load_module_function("Math", "inv_sqrt",              math_inv_sqrt);
	env.load_module_function("Math", "sin",                   math_sin);
	env.load_module_function("Math", "cos",                   math_cos);
	env.load_module_function("Math", "asin",                  math_asin);
	env.load_module_function("Math", "acos",                  math_acos);
	env.load_module_function("Math", "tan",                   math_tan);
	env.load_module_function("Math", "atan2",                 math_atan2);
	env.load_module_function("Math", "abs",                   math_abs);
	env.load_module_function("Math", "fmod",                  math_fmod);
	env.load_module_function("Math", "ray_oobb_intersection", math_ray_oobb_intersection);
}

} // namespace crown
