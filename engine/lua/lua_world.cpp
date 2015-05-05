/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "array.h"
#include "gui.h"
#include "lua_assert.h"
#include "temp_allocator.h"
#include "world.h"
#include "device.h"
#include "resource_manager.h"

namespace crown
{

static int world_spawn_unit(lua_State* L)
{
	LuaStack stack(L);
	World* world = stack.get_world(1);
	const StringId64 name = stack.get_resource_id(2);
	const Vector3& pos = stack.num_args() > 2 ? stack.get_vector3(3) : vector3::ZERO;
	const Quaternion& rot = stack.num_args() > 3 ? stack.get_quaternion(4) : quaternion::IDENTITY;

	LUA_ASSERT(device()->resource_manager()->can_get(UNIT_TYPE, name), stack, "Unit not found");

	UnitId unit = world->spawn_unit(name, pos, rot);
	stack.push_unit(world->get_unit(unit));
	return 1;
}

static int world_destroy_unit(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->destroy_unit(stack.get_unit(2)->id());
	return 0;
}

static int world_num_units(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(stack.get_world(1)->num_units());
	return 1;
}

static int world_units(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	TempAllocator1024 alloc;
	Array<UnitId> all_units(alloc);
	world->units(all_units);

	stack.push_table();
	for (uint32_t i = 0; i < array::size(all_units); i++)
	{
		stack.push_key_begin((int32_t) i + 1);
		stack.push_unit(world->get_unit(all_units[i]));
		stack.push_key_end();
	}

	return 1;
}

static int world_link_unit(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->link_unit(stack.get_unit(2)->id(), stack.get_unit(3)->id());
	return 0;
}

static int world_unlink_unit(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->unlink_unit(stack.get_unit(2)->id());
	return 0;
}

static int world_update_animations(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update_animations(stack.get_float(2));
	return 0;
}

static int world_update_scene(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update_scene(stack.get_float(2));
	return 0;
}

static int world_update(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update(stack.get_float(2));
	return 0;
}

static int world_play_sound(lua_State* L)
{
	LuaStack stack(L);
	World* world = stack.get_world(1);
	const StringId64 name = stack.get_resource_id(2);
	const int32_t nargs = stack.num_args();
	const bool loop = nargs > 2 ? stack.get_bool(3) : false;
	const float volume = nargs > 3 ? stack.get_float(4) : 1.0f;
	const Vector3& pos = nargs > 4 ? stack.get_vector3(5) : vector3::ZERO;
	const float range = nargs > 5 ? stack.get_float(6) : 1000.0f;

	LUA_ASSERT(device()->resource_manager()->can_get(SOUND_TYPE, name), stack, "Sound not found");

	stack.push_sound_instance_id(world->play_sound(name, loop, volume, pos, range));
	return 1;
}

static int world_stop_sound(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->stop_sound(stack.get_sound_instance_id(2));
	return 0;
}

static int world_link_sound(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->link_sound(stack.get_sound_instance_id(2),
		stack.get_unit(3),
		stack.get_int(4));
	return 0;
}

static int world_set_listener_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_listener_pose(stack.get_matrix4x4(2));
	return 0;
}

static int world_set_sound_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_position(stack.get_sound_instance_id(2),
		stack.get_vector3(3));
	return 0;
}

static int world_set_sound_range(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_range(stack.get_sound_instance_id(2),
		stack.get_float(3));
	return 0;
}

static int world_set_sound_volume(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_volume(stack.get_sound_instance_id(2),
		stack.get_float(3));
	return 0;
}

static int world_create_window_gui(lua_State* L)
{
	LuaStack stack(L);
	World* world = stack.get_world(1);
	GuiId id = world->create_window_gui(stack.get_int(2), stack.get_int(3), stack.get_string(4));
	stack.push_gui(world->get_gui(id));
	return 1;
}

static int world_destroy_gui(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->destroy_gui(stack.get_gui(2)->id());
	return 0;
}

static int world_create_debug_line(lua_State* L)
{
	LuaStack stack(L);
	stack.push_debug_line(stack.get_world(1)->create_debug_line(stack.get_bool(2)));
	return 1;
}

static int world_destroy_debug_line(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->destroy_debug_line(stack.get_debug_line(2));
	return 0;
}

static int world_load_level(lua_State* L)
{
	LuaStack stack(L);
	StringId64 name = stack.get_resource_id(2);
	LUA_ASSERT(device()->resource_manager()->can_get(LEVEL_TYPE, name), stack, "Level not found");
	stack.get_world(1)->load_level(name);
	return 0;
}

static int world_physics_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_physics_world(stack.get_world(1)->physics_world());
	return 1;
}

static int world_sound_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_sound_world(stack.get_world(1)->sound_world());
	return 1;
}

static int world_tostring(lua_State* L)
{
	LuaStack stack(L);
	World* w = stack.get_world(1);
	stack.push_fstring("World (%p)", w);
	return 1;
}

void load_world(LuaEnvironment& env)
{
	env.load_module_function("World", "spawn_unit",         world_spawn_unit);
	env.load_module_function("World", "destroy_unit",       world_destroy_unit);
	env.load_module_function("World", "num_units",          world_num_units);
	env.load_module_function("World", "units",              world_units);
	env.load_module_function("World", "link_unit",          world_link_unit);
	env.load_module_function("World", "unlink_unit",        world_unlink_unit);
	env.load_module_function("World", "update_animations",  world_update_animations);
	env.load_module_function("World", "update_scene",       world_update_scene);
	env.load_module_function("World", "update",             world_update);
	env.load_module_function("World", "play_sound",         world_play_sound);
	env.load_module_function("World", "stop_sound",         world_stop_sound);
	env.load_module_function("World", "link_sound",         world_link_sound);
	env.load_module_function("World", "set_listener_pose",  world_set_listener_pose);
	env.load_module_function("World", "set_sound_position", world_set_sound_position);
	env.load_module_function("World", "set_sound_range",    world_set_sound_range);
	env.load_module_function("World", "set_sound_volume",   world_set_sound_volume);
	env.load_module_function("World", "create_window_gui",  world_create_window_gui);
	env.load_module_function("World", "destroy_gui",        world_destroy_gui);
	env.load_module_function("World", "create_debug_line",  world_create_debug_line);
	env.load_module_function("World", "destroy_debug_line", world_destroy_debug_line);
	env.load_module_function("World", "load_level",         world_load_level);
	env.load_module_function("World", "physics_world",      world_physics_world);
	env.load_module_function("World", "sound_world",        world_sound_world);
	env.load_module_function("World", "__index",            "World");
	env.load_module_function("World", "__tostring",         world_tostring);
}

} // namespace crown
