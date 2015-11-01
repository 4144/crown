/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "int_setting.h"
#include "float_setting.h"
#include "string_setting.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int int_setting_query_value(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	IntSetting* setting = IntSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	stack.push_int32(setting->value());
	return 1;
}

static int int_setting_query_synopsis(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	IntSetting* setting = IntSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	stack.push_string(setting->synopsis());
	return 1;
}

static int int_setting_query_min(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	IntSetting* setting = IntSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	stack.push_int32(setting->min());
	return 1;
}

static int int_setting_query_max(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	IntSetting* setting = IntSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	stack.push_int32(setting->max());
	return 1;
}

static int int_setting_update(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	const int32_t setting_value = stack.get_int(2);
	IntSetting* setting = IntSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	*setting = setting_value;
	return 0;
}

static int float_setting_value(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	FloatSetting* setting = FloatSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	stack.push_float(setting->value());
	return 1;
}

static int float_setting_synopsis(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	FloatSetting* setting = FloatSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	stack.push_string(setting->synopsis());
	return 1;
}

static int float_setting_min(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	FloatSetting* setting = FloatSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	stack.push_float(setting->min());
	return 1;
}

static int float_setting_max(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	FloatSetting* setting = FloatSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	stack.push_float(setting->max());
	return 1;
}

static int float_setting_update(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	const float setting_value = stack.get_float(2);
	FloatSetting* setting = FloatSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	*setting = setting_value;
	return 0;
}

static int string_setting_value(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	StringSetting* setting = StringSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	stack.push_string(setting->value());
	return 1;
}

static int string_setting_synopsis(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	StringSetting* setting = StringSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	stack.push_string(setting->synopsis());
	return 1;
}

static int string_setting_update(lua_State* L)
{
	LuaStack stack(L);
	const char* setting_name = stack.get_string(1);
	const char* setting_value = stack.get_string(2);
	StringSetting* setting = StringSetting::find_setting(setting_name);
	LUA_ASSERT(setting != NULL, stack, "Setting not found");
	*setting = setting_value;
	return 0;
}

void load_settings(LuaEnvironment& env)
{
	env.load_module_function("IntSetting", "value",    int_setting_query_value);
	env.load_module_function("IntSetting", "synopsis", int_setting_query_synopsis);
	env.load_module_function("IntSetting", "min",      int_setting_query_min);
	env.load_module_function("IntSetting", "max",      int_setting_query_max);
	env.load_module_function("IntSetting", "update",   int_setting_update);

	env.load_module_function("FloatSetting", "value",    float_setting_value);
	env.load_module_function("FloatSetting", "synopsis", float_setting_synopsis);
	env.load_module_function("FloatSetting", "min",      float_setting_min);
	env.load_module_function("FloatSetting", "max",      float_setting_max);
	env.load_module_function("FloatSetting", "update",   float_setting_update);

	env.load_module_function("StringSetting", "value",    string_setting_value);
	env.load_module_function("StringSetting", "synopsis", string_setting_synopsis);
	env.load_module_function("StringSetting", "update",   string_setting_update);
}

} // namespace crown
