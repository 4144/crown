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

#include "lua_stack.h"
#include "lua_environment.h"
#include "sound_world.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int sound_world_stop_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->stop_all();
	return 0;
}

//-----------------------------------------------------------------------------
static int sound_world_pause_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->pause_all();
	return 0;
}

//-----------------------------------------------------------------------------
static int sound_world_resume_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->resume_all();
	return 0;
}

//-----------------------------------------------------------------------------
static int sound_world_is_playing(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_sound_world(1)->is_playing(stack.get_sound_instance_id(2)));
	return 1;
}

//-----------------------------------------------------------------------------
static int sound_world_tostring(lua_State* L)
{
	LuaStack stack(L);
	SoundWorld* sw = stack.get_sound_world(1);
	stack.push_fstring("SoundWorld (%p)", sw);
	return 1;
}

//-----------------------------------------------------------------------------
void load_sound_world(LuaEnvironment& env)
{
	env.load_module_function("SoundWorld", "stop_all",   sound_world_stop_all);
	env.load_module_function("SoundWorld", "pause_all",  sound_world_pause_all);
	env.load_module_function("SoundWorld", "resume_all", sound_world_resume_all);
	env.load_module_function("SoundWorld", "is_playing", sound_world_is_playing);
	env.load_module_function("SoundWorld", "__index",    "SoundWorld");
	env.load_module_function("SoundWorld", "__tostring", sound_world_tostring);
}

} // namespace crown
