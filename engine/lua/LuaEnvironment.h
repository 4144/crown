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

#include "lua.hpp"
#include "Config.h"
#include "Types.h"

namespace crown
{

enum LuaArgumentType
{
	ARGUMENT_FLOAT
};

class LuaResource;

/// LuaEnvironment is a wrapper of a subset of Lua functions and 
/// provides utilities for extending Lua
class LuaEnvironment
{

public:
	/// Constructor
							LuaEnvironment();
	/// Init Lua state and open libraries. Must be called first
	void					init();
	/// Close Lua state and shutdown LuaEnvironment
	void					shutdown();

	const char*				error();

	/// Loads and execute the given @a lr lua script resource.
	void					load(const LuaResource* lr);

	/// Calls the global function @a func with @a argc argument number.
	/// Each argument is a pair (type, value).
	/// Example call:
	/// call_global("myfunc", 1, ARGUMENT_FLOAT, 3.14f)
	void					call_global(const char* func, uint8_t argc, ...);

	/// Load a function which will be used in Lua. @a module is the name of table contenitor,
	/// @a name is the name of function in module and @func is the pointer to the function.
	/// _func_ must be a C/lua function (__int32_t function_name(lua_State* L)__)
	void					load_module_function(const char* module, const char* name, const lua_CFunction func);

	/// Load a enum's value which will be used in Lua. 
	/// @a module is the name of table contenitor, generally take  enum's name
	/// @a name is module's name that refears _value_ and @value is an unsigned integer
	void					load_module_enum(const char* module, const char* name, uint32_t value);

private:

	void					lua_error();
	// Disable copying
							LuaEnvironment(const LuaEnvironment&);
	LuaEnvironment& 		operator=(const LuaEnvironment&);

private:
	/// Required by each Lua function
	lua_State*				m_state;

	/// LuaEnvironment is used right now?
	bool					m_is_used;

	char					m_error_buffer[1024];
	char					m_tmp_buffer[1024];
	static const char* 		class_system;
	static const char*		commands_list;
	static const char*		get_cmd_by_name;
	static const char*		tmp_print_table;
	static const char*		count_all;
};


void load_int_setting(LuaEnvironment& env);
void load_float_setting(LuaEnvironment& env);
void load_string_setting(LuaEnvironment& env);

void load_vec2(LuaEnvironment& env);
void load_vec3(LuaEnvironment& env);
void load_mat4(LuaEnvironment& env);
void load_quat(LuaEnvironment& env);
void load_math(LuaEnvironment& env);
void load_mouse(LuaEnvironment& env);
void load_keyboard(LuaEnvironment& env);
void load_touch(LuaEnvironment& env);
void load_accelerometer(LuaEnvironment& env);
void load_camera(LuaEnvironment& env);
void load_device(LuaEnvironment& env);
void load_window(LuaEnvironment& env);

} // namespace crown