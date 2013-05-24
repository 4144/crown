#include "LuaStack.h"
#include <cassert>

namespace crown
{

static const int32_t 	LUA_VEC2_BUFFER_SIZE = 4096;
static Vec2 			vec2_buffer[LUA_VEC2_BUFFER_SIZE];
static uint32_t 		vec2_used = 0;

static const int32_t 	LUA_VEC3_BUFFER_SIZE = 4096;
static Vec3 			vec3_buffer[LUA_VEC3_BUFFER_SIZE];
static uint32_t 		vec3_used = 0;

static const int32_t 	LUA_MAT4_BUFFER_SIZE = 4096;
static Mat4 			mat4_buffer[LUA_MAT4_BUFFER_SIZE];
static uint32_t 		mat4_used = 0;

static const int32_t 	LUA_QUAT_BUFFER_SIZE = 4096;
static Quat 			quat_buffer[LUA_QUAT_BUFFER_SIZE];
static uint32_t 		quat_used = 0;

//-----------------------------------------------------------------------------
Vec2* next_vec2()
{
	assert(vec2_used < LUA_VEC2_BUFFER_SIZE);

	return &vec2_buffer[vec2_used++];
}

//-----------------------------------------------------------------------------
Vec3* next_vec3()
{
	assert(vec3_used < LUA_VEC3_BUFFER_SIZE);

	return &vec3_buffer[vec3_used++];
}

//-----------------------------------------------------------------------------
Mat4* next_mat4()
{
	assert(mat4_used < LUA_MAT4_BUFFER_SIZE);

	return &mat4_buffer[mat4_used++];
}

//-----------------------------------------------------------------------------
Quat* next_quat()
{
	assert(quat_used < LUA_QUAT_BUFFER_SIZE);

	return &quat_buffer[quat_used++];
}

//-----------------------------------------------------------------------------	
LuaStack::LuaStack(lua_State* L)
{
	m_state = L;
}

//-----------------------------------------------------------------------------
void LuaStack::push_bool(bool value)
{
	lua_pushboolean(m_state, value);
}

//-----------------------------------------------------------------------------
void LuaStack::push_int(int32_t value)
{
	lua_pushinteger(m_state, value);
}

//-----------------------------------------------------------------------------
void LuaStack::push_float(float value)
{
	lua_pushnumber(m_state, value);
}

//-----------------------------------------------------------------------------
void LuaStack::push_string(const char* str, size_t len)
{
	lua_pushlstring(m_state, str, len);
}

//-----------------------------------------------------------------------------
void LuaStack::push_vec2(Vec2* v)
{
	assert(v >= &vec2_buffer[0] && v <= &vec2_buffer[LUA_VEC2_BUFFER_SIZE-1]);

	lua_pushlightuserdata(m_state, v);
}

//-----------------------------------------------------------------------------
void LuaStack::push_vec3(Vec3* v)
{
	assert(v >= &vec3_buffer[0] && v <= &vec3_buffer[LUA_VEC3_BUFFER_SIZE-1]);

	lua_pushlightuserdata(m_state, v);
}

//-----------------------------------------------------------------------------
void LuaStack::push_mat4(Mat4* m)
{
	assert(m >= &mat4_buffer[0] && m <= &mat4_buffer[LUA_MAT4_BUFFER_SIZE-1]);

	lua_pushlightuserdata(m_state, m);
}

//-----------------------------------------------------------------------------
void LuaStack::push_quat(Quat* q)
{
	assert(q >= &quat_buffer[0] && q <= &quat_buffer[LUA_MAT4_BUFFER_SIZE-1]);

	lua_pushlightuserdata(m_state, q);
}

//-----------------------------------------------------------------------------
bool LuaStack::get_bool(int32_t index)
{
	return (bool) luaL_checkinteger(m_state, index);
}

//-----------------------------------------------------------------------------
int32_t LuaStack::get_int(int32_t index)
{
	return luaL_checkinteger(m_state, index);
}

//-----------------------------------------------------------------------------
float LuaStack::get_float(int32_t index)
{
	return luaL_checknumber(m_state, index);
}

//-----------------------------------------------------------------------------
const char* LuaStack::get_string(int32_t index)
{
	return luaL_checkstring(m_state, index);
}

//-----------------------------------------------------------------------------
Vec2* LuaStack::get_vec2(int32_t index)
{
	assert(lua_islightuserdata(m_state, index));

	Vec2* v = (Vec2*)lua_touserdata(m_state, index);

	assert(v >= &vec2_buffer[0] && v <= &vec2_buffer[LUA_VEC2_BUFFER_SIZE-1]);

	return v;
}

//-----------------------------------------------------------------------------
Vec3* LuaStack::get_vec3(int32_t index)
{
	assert(lua_islightuserdata(m_state, index));

	Vec3* v = (Vec3*)lua_touserdata(m_state, index);

	assert(v >= &vec3_buffer[0] && v <= &vec3_buffer[LUA_VEC3_BUFFER_SIZE-1]);

	return v;
}

//-----------------------------------------------------------------------------
Mat4* LuaStack::get_mat4(int32_t index)
{
	assert(lua_islightuserdata(m_state, index));

	Mat4* m = (Mat4*)lua_touserdata(m_state, index);

	assert(m >= &mat4_buffer[0] && m <= &mat4_buffer[LUA_MAT4_BUFFER_SIZE-1]);

	return m;
}

//-----------------------------------------------------------------------------
Quat* LuaStack::get_quat(int32_t index)
{
	assert(lua_islightuserdata(m_state, index));

	Quat* q = (Quat*)lua_touserdata(m_state, index);

	assert(q >= &quat_buffer[0] && q <= &quat_buffer[LUA_QUAT_BUFFER_SIZE-1]);

	return q;
}

} // namespace crown