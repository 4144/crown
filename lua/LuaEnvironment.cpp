#include <cassert>
#include "LuaEnvironment.h"


namespace crown
{

LuaEnvironment::LuaEnvironment(lua_State* L) :
	m_state(L)
{

}

//-----------------------------------------------------------
void LuaEnvironment::load_module_function(const char* module, const char* name, const lua_CFunction func)
{
	luaL_Reg entry[2];

	entry[0].name = name;
	entry[0].func = func;

	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(m_state, module, entry);
}

extern "C"
{
	int luaopen_libcrownlua(lua_State* L)
	{
		LuaEnvironment env(L);

		load_vec2(env);
	}
}

} // namespace crown