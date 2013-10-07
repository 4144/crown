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

#include "LuaStack.h"
#include "LuaEnvironment.h"
#include "World.h"
#include "Device.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int world_spawn_unit(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	const char* name = stack.get_string(2);

	const Vec3& pos = stack.num_args() > 2 ? stack.get_vec3(3) : Vec3::ZERO;
	const Quat& rot = stack.num_args() > 3 ? stack.get_quat(4) : Quat::IDENTITY;

	UnitId unit = world->spawn_unit(name, pos, rot);

	stack.push_unit(world->lookup_unit(unit));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int world_mesh(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);

	stack.push_mesh(world->mesh());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int world_play_sound(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);
	const char* name = stack.get_string(2);

	const bool loop = stack.num_args() > 2 ? stack.get_bool(3) : false;
	const float volume = stack.num_args() > 3 ? stack.get_float(4) : 1.0f; // test value
	const Vec3& pos = stack.num_args() > 4 ? stack.get_vec3(5) : Vec3::ZERO;
	const float range = stack.num_args() > 5 ? stack.get_float(6) : 1000.0f; // test value

	SoundInstanceId id = world->play_sound(name, loop, volume, pos, range);

	stack.push_int32(id.encode());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int world_pause_sound(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);

	SoundInstanceId id;
	id.decode(stack.get_int(2));

	world->pause_sound(id);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int world_link_sound(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);

	SoundInstanceId sound;
	sound.decode(stack.get_int(2));

	UnitId unit;
	unit.decode(stack.get_int(3));

	world->link_sound(sound, unit);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int world_set_listener(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);

	const Vec3& pos = stack.get_vec3(2);
	const Vec3& vel = stack.get_vec3(3);
	const Vec3& or_up = stack.get_vec3(4);
	const Vec3& or_at = stack.get_vec3(5);

	world->set_listener(pos, vel, or_up, or_at);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int world_set_sound_position(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);

	SoundInstanceId id;
	id.decode(stack.get_int(2));

	const Vec3& pos = stack.get_vec3(3);

	world->set_sound_position(id, pos);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int world_set_sound_range(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);

	SoundInstanceId id;
	id.decode(stack.get_int(2));

	float range = stack.get_float(3);

	world->set_sound_range(id, range);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int world_set_sound_volume(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);

	SoundInstanceId id;
	id.decode(stack.get_int(2));
	float vol = stack.get_float(3);

	world->set_sound_volume(id, vol);

	return 0;
}

//-----------------------------------------------------------------------------
void load_world(LuaEnvironment& env)
{
	env.load_module_function("World", "spawn_unit",			world_spawn_unit);

	env.load_module_function("World", "play_sound",			world_play_sound);
	env.load_module_function("World", "pause_sound", 		world_pause_sound);
	env.load_module_function("World", "link_sound",			world_link_sound);
	env.load_module_function("World", "set_listener", 		world_set_listener);
	env.load_module_function("World", "set_sound_position", world_set_sound_position);
	env.load_module_function("World", "set_sound_range", 	world_set_sound_range);
	env.load_module_function("World", "set_sound_volume", 	world_set_sound_volume);

	// GARBAGE
	env.load_module_function("World", "mesh", world_mesh);
}

} // namespace crown