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

#include "Device.h"
#include "ResourcePackage.h"
#include "World.h"
#include "WorldManager.h"
#include "LuaEnvironment.h"
#include "LuaStack.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int device_argv(lua_State* L)
{
	LuaStack stack(L);

	const int32_t argc = device()->argc();
	const char** argv = device()->argv();

	stack.push_table();
	for (int32_t i = 0; i < argc; i++)
	{
		// Be friendly to Lua's one-indexed arrays
		stack.push_key_begin(i + 1);
			stack.push_string(argv[i]);
		stack.push_key_end();
	}

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_frame_count(lua_State* L)
{
	LuaStack stack(L);

	uint64_t frame = device()->frame_count();

	stack.push_uint64(frame);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_last_delta_time(lua_State* L)
{
	LuaStack stack(L);

	float delta = device()->last_delta_time();

	stack.push_float(delta);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_start(lua_State* /*L*/)
{
	device()->start();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_stop(lua_State* /*L*/)
{
	device()->stop();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_create_world(lua_State* L)
{
	LuaStack stack(L);
	const WorldId world_id = device()->create_world();
	stack.push_world(device()->world_manager()->lookup_world(world_id));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_destroy_world(lua_State* L)
{
	LuaStack stack(L);
	device()->destroy_world(stack.get_world(1)->id());
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_update_world(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	const float dt = stack.get_float(2);

	device()->update_world(world, dt);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_render_world(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	Camera* camera = stack.get_camera(2);

	device()->render_world(world, camera);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_create_resource_package(lua_State* L)
{
	LuaStack stack(L);

	const char* package = stack.get_string(1);
	stack.push_resource_package(device()->create_resource_package(package));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_destroy_resource_package(lua_State* L)
{
	LuaStack stack(L);

	ResourcePackage* package = stack.get_resource_package(1);
	device()->destroy_resource_package(package);

	return 0;
}

//-----------------------------------------------------------------------------
void load_device(LuaEnvironment& env)
{
	env.load_module_function("Device", "argv",                     device_argv);
	env.load_module_function("Device", "frame_count",              device_frame_count);
	env.load_module_function("Device", "last_delta_time",          device_last_delta_time);
	env.load_module_function("Device", "start",                    device_start);
	env.load_module_function("Device", "stop",                     device_stop);
	env.load_module_function("Device", "create_world",             device_create_world);
	env.load_module_function("Device", "destroy_world",            device_destroy_world);
	env.load_module_function("Device", "update_world",             device_update_world);
	env.load_module_function("Device", "render_world",             device_render_world);
	env.load_module_function("Device", "create_resource_package",  device_create_resource_package);
	env.load_module_function("Device", "destroy_resource_package", device_destroy_resource_package);
}

} // namespace crown
