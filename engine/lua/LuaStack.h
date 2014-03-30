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
#include "Types.h"
#include "LuaSystem.h"
#include "Vector3.h"
#include "Vector2.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include "StringUtils.h"

namespace crown
{

class PhysicsWorld;
class SoundWorld;
class World;
struct Actor;
struct Camera;
struct Controller;
struct Gui;
struct Mesh;
struct ResourcePackage;
struct Sprite;
struct Unit;
struct DebugLine;
struct Raycast;

typedef Id SoundInstanceId;
typedef Id GuiId;
typedef Id GuiComponentId;

typedef int (*MetamethodFunction)(lua_State*);

void clear_lua_temporaries();

class LuaStack
{
public:

	//-----------------------------------------------------------------------------	
	LuaStack(lua_State* L)
		: m_state(L)
	{
	}

	//-----------------------------------------------------------------------------
	lua_State* state()
	{
		return m_state;
	}

	/// Returns the number of elements in the stack.
	/// When called inside a function, it can be used to count
	/// the number of arguments passed to the function itself.
	int32_t num_args()
	{
		return lua_gettop(m_state);
	}

	//-----------------------------------------------------------------------------
	void push_nil()
	{
		lua_pushnil(m_state);
	}

	//-----------------------------------------------------------------------------
	void push_bool(bool value)
	{
		lua_pushboolean(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_int32(int32_t value)
	{
		lua_pushinteger(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_uint32(uint32_t value)
	{
		lua_pushinteger(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_int64(int64_t value)
	{
		lua_pushinteger(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_uint64(uint64_t value)
	{
		lua_pushinteger(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_float(float value)
	{
		lua_pushnumber(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_string(const char* s)
	{
		lua_pushstring(m_state, s);
	}

	//-----------------------------------------------------------------------------
	void push_literal(const char* s, size_t len)
	{
		lua_pushlstring(m_state, s, len);
	}

	//-----------------------------------------------------------------------------
	bool get_bool(int32_t index)
	{
		return (bool) lua_toboolean(m_state, index);
	}

	//-----------------------------------------------------------------------------
	int32_t get_int(int32_t index)
	{
		return luaL_checkinteger(m_state, index);
	}

	//-----------------------------------------------------------------------------
	float get_float(int32_t index)
	{
		return luaL_checknumber(m_state, index);
	}

	//-----------------------------------------------------------------------------
	const char* get_string(int32_t index)
	{
		return luaL_checkstring(m_state, index);
	}

	/// Pushes an empty table onto the stack.
	/// When you want to set keys on the table, you have to use LuaStack::push_key_begin()
	/// and LuaStack::push_key_end() as in the following example:
	///
	/// LuaStack stack(L)
	/// stack.push_table()
	/// stack.push_key_begin("foo"); stack.push_foo(); stack.push_key_end()
	/// stack.push_key_begin("bar"); stack.push_bar(); stack.push_key_end()
	/// return 1;
	void push_table()
	{
		lua_newtable(m_state);
	}

	/// See Stack::push_table()
	void push_key_begin(const char* key)
	{
		lua_pushstring(m_state, key);
	}

	/// See Stack::push_table()
	void push_key_begin(int32_t i)
	{
		lua_pushnumber(m_state, i);
	}

	/// See Stack::push_table()
	void push_key_end()
	{
		lua_settable(m_state, -3);
	}

	/// Pushes an empty metatable onto the stack.
	/// When you want to associate a metamethod to a function, you have to use 
	/// LuaStack::set_metamethod_function(const char* key, MetamethodFunction func).
	/// Call LuaStack::end_metatable() to associate this metatable to userdata of any kind.
	///
	/// stack.push_userdata(); // Could be Vector3Box or a table
	/// stack.begin_metatable("Foo_mt");
	/// stack.set_metamethod_function("__index", function_ptr);
	/// stack.set_metamethod_function("__newindex", another_function_ptr);
	/// stack.end_metatable();
	void create_metatable(const char* name)
	{
		luaL_newmetatable(m_state, name);
	}

	/// See LuaStack::begin_metatable()
	void set_metatable_function(const char* key, MetamethodFunction func)
	{
		lua_pushcfunction(m_state, func);
		lua_setfield(m_state, -2, key);
	}

	///	Sets table on top of stack to metafield @a key of another table.
	/// Only '__index' and '__newindex' metamethods are available.
	void set_metatable_table(const char* key)
	{
		CE_ASSERT(string::strcmp(key, "__index") == 0 || string::strcmp(key, "__newindex") == 0, "Illegal metamethod");
		lua_setfield(m_state, -2, key);		
	}

	/// Sets table's __index metamethod to its self
	void set_self_index()
	{
		lua_pushvalue(m_state, -1);
		lua_setfield(m_state, -2, "__index");
	}

	/// See LuaStack::begin_metatable()
	void set_metatable()
	{
		lua_setmetatable(m_state, -2);
	}

	///
	void get_global(const char* name)
	{
		lua_getglobal(m_state, name);
	}

	///
	void get_global_metatable(const char* name)
	{
		luaL_getmetatable(m_state, name);
	}

	//-----------------------------------------------------------------------------
	void push_resource_package(ResourcePackage* package)
	{
		lua_pushlightuserdata(m_state, package);
	}

	//-----------------------------------------------------------------------------
	ResourcePackage* get_resource_package(int32_t index)
	{
		return (ResourcePackage*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_world(World* world)
	{
		lua_pushlightuserdata(m_state, world);
	}

	//-----------------------------------------------------------------------------
	World* get_world(int32_t index)
	{
		return (World*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_sound_world(SoundWorld* sw)
	{
		lua_pushlightuserdata(m_state, sw);
	}

	//-----------------------------------------------------------------------------
	SoundWorld* get_sound_world(int32_t index)
	{
		return (SoundWorld*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_unit(Unit* unit)
	{
		lua_pushlightuserdata(m_state, unit);
	}

	//-----------------------------------------------------------------------------
	Unit* get_unit(int32_t index)
	{
		return (Unit*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_camera(Camera* camera)
	{
		lua_pushlightuserdata(m_state, camera);
	}

	//-----------------------------------------------------------------------------
	Camera* get_camera(int32_t index)
	{
		return (Camera*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_mesh(Mesh* mesh)
	{
		lua_pushlightuserdata(m_state, mesh);
	}

	//-----------------------------------------------------------------------------
	Mesh* get_mesh(int32_t index)
	{
		return (Mesh*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_sprite(Sprite* sprite)
	{
		lua_pushlightuserdata(m_state, sprite);
	}

	//-----------------------------------------------------------------------------
	Sprite* get_sprite(int32_t index)
	{
		return (Sprite*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	PhysicsWorld* get_physics_world(int32_t index)
	{
		return (PhysicsWorld*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_physics_world(PhysicsWorld* world)
	{
		lua_pushlightuserdata(m_state, world);
	}

	//-----------------------------------------------------------------------------
	void push_actor(Actor* actor)
	{
		lua_pushlightuserdata(m_state, actor);
	}

	//-----------------------------------------------------------------------------
	Actor* get_actor(int32_t index)
	{
		return (Actor*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_controller(Controller* controller)
	{
		lua_pushlightuserdata(m_state, controller);
	}

	//-----------------------------------------------------------------------------
	Controller* get_controller(int32_t index)
	{
		return (Controller*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_raycast(Raycast* raycast)
	{
		lua_pushlightuserdata(m_state, raycast);
	}

	//-----------------------------------------------------------------------------
	Raycast* get_raycast(int32_t index)
	{
		return (Raycast*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_sound_instance_id(const SoundInstanceId id)
	{
		uintptr_t enc = id.encode();
		lua_pushlightuserdata(m_state, (void*)enc);
	}

	//-----------------------------------------------------------------------------
	SoundInstanceId get_sound_instance_id(int32_t index)
	{
		uint32_t enc = (uintptr_t) lua_touserdata(m_state, index);
		SoundInstanceId id;
		id.decode(enc);
		return id;
	}

	//-----------------------------------------------------------------------------
	void push_gui(Gui* gui)
	{
		lua_pushlightuserdata(m_state, gui);
	}

	//-----------------------------------------------------------------------------
	Gui* get_gui(int32_t index)
	{
		return (Gui*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_gui_component_id(GuiComponentId id)
	{
		uintptr_t enc = id.encode();
		lua_pushlightuserdata(m_state, (void*)enc);		
	}

	//-----------------------------------------------------------------------------
	GuiComponentId get_gui_component_id(int32_t index)
	{
		uint32_t enc = (uintptr_t) lua_touserdata(m_state, index);
		GuiComponentId id;
		id.decode(enc);
		return id;	
	}

	//-----------------------------------------------------------------------------
	void push_debug_line(DebugLine* line)
	{
		lua_pushlightuserdata(m_state, line);
	}

	//-----------------------------------------------------------------------------
	DebugLine* get_debug_line(int32_t index)
	{
		return (DebugLine*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	Vector2& get_vector2(int32_t index)
	{
		void* v = lua_touserdata(m_state, index);

		if (!lua_system::is_vector2(index))
		{
			luaL_typerror(m_state, index, "Vector2");
		}

		return *(Vector2*)v;
	}

	//-----------------------------------------------------------------------------
	Vector3& get_vector3(int32_t index)
	{
		void* v = lua_touserdata(m_state, index);

		if (!lua_system::is_vector3(index))
		{
			luaL_typerror(m_state, index, "Vector3");
		}

		return *(Vector3*)v;
	}

	//-----------------------------------------------------------------------------
	Matrix4x4& get_matrix4x4(int32_t index)
	{
		void* m = lua_touserdata(m_state, index);

		if (!lua_system::is_matrix4x4(index))
		{
			luaL_typerror(m_state, index, "Matrix4x4");
		}

		return *(Matrix4x4*)m;
	}

	//-----------------------------------------------------------------------------
	Quaternion& get_quaternion(int32_t index)
	{
		void* q = lua_touserdata(m_state, index);

		if (!lua_system::is_quaternion(index))
		{
			luaL_typerror(m_state, index, "Quaternion");
		}

		return *(Quaternion*)q;
	}

	//-----------------------------------------------------------------------------
	void push_vector2(const Vector2& v)
	{
		lua_pushlightuserdata(m_state, lua_system::next_vector2(v));
	}

	//-----------------------------------------------------------------------------
	void push_vector3(const Vector3& v)
	{
		lua_pushlightuserdata(m_state, lua_system::next_vector3(v));
	}

	//-----------------------------------------------------------------------------
	void push_matrix4x4(const Matrix4x4& m)
	{
		lua_pushlightuserdata(m_state, lua_system::next_matrix4x4(m));
	}

	//-----------------------------------------------------------------------------
	void push_quaternion(const Quaternion& q)
	{
		lua_pushlightuserdata(m_state, lua_system::next_quaternion(q));
	}

	/// Allocates a new Vector3Box (full userdata)
	Vector3& push_vector3box()
	{
		return *(Vector3*) lua_newuserdata(m_state, sizeof(Vector3));
	}

	/// Retrieves Vector3Box (full userdata)
	Vector3& get_vector3box(uint32_t index)
	{
		CE_ASSERT(lua_isuserdata(m_state, index), "Unable to retrieve 'Vector3Box' (wrong index)");
		Vector3* ud = (Vector3*) luaL_checkudata(m_state, index, "Vector3Box_i_mt");
		return *ud;
	}

	/// Allocates a new QuaternionBox (full userdata)
	Quaternion& push_quaternionbox()
	{
		return *(Quaternion*) lua_newuserdata(m_state, sizeof(Quaternion));
	}

	/// Retrieves QuaternionBox (full userdata)
	Quaternion& get_quaternionbox(uint32_t index)
	{
		CE_ASSERT(lua_isuserdata(m_state, index), "Unable to retrieve QuaternionBox (wrong index)");
		Quaternion* ud = (Quaternion*) luaL_checkudata(m_state, index, "QuaternionBox_i_mt");
		return *ud;
	}

	/// Allocates a new Matrix4x4Box (full userdata)
	Matrix4x4& push_matrix4x4box()
	{
		return *(Matrix4x4*) lua_newuserdata(m_state, sizeof(Matrix4x4));
	}

	/// Retrieves QuaternionBox (full userdata)
	Matrix4x4& get_matrix4x4box(uint32_t index)
	{
		CE_ASSERT(lua_isuserdata(m_state, index), "Unable to retrieve Matrix4x4Box (wrong index)");
		Matrix4x4* ud = (Matrix4x4*) luaL_checkudata(m_state, index, "Matrix4x4Box_i_mt");
		return *ud;
	}

private:

	lua_State* m_state;
};

} // namespace crown