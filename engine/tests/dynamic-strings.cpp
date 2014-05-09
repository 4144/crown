#include "Crown.h"

using namespace crown;

//-----------------------------------------------------------------------------
void creation_test()
{
	DynamicString string(default_allocator(), "Creation Test");
	CE_LOGI("%s", string.c_str());

	string = "Creation Test OK!";
	CE_LOGI("%s", string.c_str());
}

//-----------------------------------------------------------------------------
void equal_test()
{
	DynamicString string(default_allocator(), "Equal Test");
	CE_LOGI("%s", string.c_str());

	DynamicString string1(default_allocator(), "DynamicString assigned!");	

	string = string1;
	CE_LOGI("%s", string.c_str());

	string = "C-string assigned";
	CE_LOGI("%s", string.c_str());

	string = 'C';
	CE_LOGI("%s", string.c_str());

	string = "Equal Test OK!";
	CE_LOGI("%s", string.c_str());
}

//-----------------------------------------------------------------------------
void plus_equal_test()
{
	DynamicString string(default_allocator(), "PlusEqual Test");
	DynamicString string1(default_allocator(), " DynamicString appended!");

	string += string1;
	CE_LOGI("%s", string.c_str());

	string += " C-string appended! ";
	CE_LOGI("%s", string.c_str());

	string += 'C';
	CE_LOGI("%s", string.c_str());

	string = "PlusEqual Test OK!";
	CE_LOGI("%s", string.c_str());
}

//-----------------------------------------------------------------------------
void starts_ends_with_test()
{
	TempAllocator1024 alloc;

	DynamicString string(alloc, "lua/game.lua");

	CE_ASSERT(string.starts_with("lua"), "FAIL");
	CE_ASSERT(string.starts_with("lua/game.lua"), "FAIL");
	CE_ASSERT(!string.starts_with("game"), "FAIL");
	CE_ASSERT(!string.starts_with("lua/game.lua/foo"), "FAIL");

	CE_ASSERT(string.ends_with(".lua"), "FAIL");
	CE_ASSERT(string.ends_with("lua/game.lua"), "FAIL");
	CE_ASSERT(!string.ends_with("foo"), "FAIL");
	CE_ASSERT(!string.ends_with("lua/game.lua/"), "FAIL");
}

int main()
{
	creation_test();
	equal_test();
	plus_equal_test();
	starts_ends_with_test();

	return 0;
}