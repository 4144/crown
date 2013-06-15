#pragma once

#include "lua.hpp"
#include "Types.h"

namespace crown
{

class LuaEnvironment
{

public:
	/// Constructor
					LuaEnvironment(lua_State* L);
	/// Load a function to proper module
	void			load_module_function(const char* module, const char* name, const lua_CFunction func);
	/// Create library based on each module which will be opened by luaopen_*
	void			create_module_library();

private:

	lua_State*		m_state;
};

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

extern "C"
{
	int32_t luaopen_libcrownlua(lua_State* L);
}



} // namespace crown