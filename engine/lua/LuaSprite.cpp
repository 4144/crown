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

#include "Sprite.h"
#include "Quaternion.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int sprite_local_position(lua_State* L)
{
	LuaStack stack(L);

	Sprite* sprite = stack.get_sprite(1);

	stack.push_vector3(sprite->local_position());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int sprite_local_rotation(lua_State* L)
{
	LuaStack stack(L);

	Sprite* sprite = stack.get_sprite(1);

	stack.push_quaternion(sprite->local_rotation());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int sprite_local_pose(lua_State* L)
{
	LuaStack stack(L);

	Sprite* sprite = stack.get_sprite(1);

	stack.push_matrix4x4(sprite->local_pose());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int sprite_set_local_position(lua_State* L)
{
	LuaStack stack(L);

	Sprite* sprite = stack.get_sprite(1);
	Vector3 pos = stack.get_vector3(2);

	sprite->set_local_position(pos);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int sprite_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);

	Sprite* sprite = stack.get_sprite(1);
	Quaternion rot = stack.get_quaternion(2);

	sprite->set_local_rotation(pos);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int sprite_set_local_pose(lua_State* L)
{
	LuaStack stack(L);

	Sprite* sprite = stack.get_sprite(1);
	Matrix4x4 pose = stack.get_matrix4x4(2);

	sprite->set_local_pose(pose);

	return 0;
}
//-----------------------------------------------------------------------------
void load_mesh(LuaEnvironment& env)
{
	env.load_module_function("Sprite", "local_position", 		sprite_local_position);
	env.load_module_function("Sprite", "local_rotation", 		sprite_local_rotation);
	env.load_module_function("Sprite", "local_pose", 			sprite_local_pose);
	env.load_module_function("Sprite", "set_local_position", 	sprite_set_local_position);
	env.load_module_function("Sprite", "set_local_rotation", 	sprite_set_local_rotation);
	env.load_module_function("Sprite", "set_local_pose", 		sprite_set_local_pose);
}

} // namespace crown