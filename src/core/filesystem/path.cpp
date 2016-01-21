/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "path.h"
#include <ctype.h> // isalpha
#include <string.h> // strlen, strrchr

namespace crown
{
namespace path
{
	bool is_absolute(const char* path)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
#if CROWN_PLATFORM_POSIX
		return strlen32(path) > 0 && path[0] == PATH_SEPARATOR;
#elif CROWN_PLATFORM_WINDOWS
		return strlen32(path) > 2 && isalpha(path[0]) && path[1] == ':' && path[2] == PATH_SEPARATOR;
#endif
	}

	bool is_root(const char* path)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
#if CROWN_PLATFORM_POSIX
		return is_absolute(path) && strlen32(path) == 1;
#elif CROWN_PLATFORM_WINDOWS
		return is_absolute(path) && strlen32(path) == 3;
#endif
	}

	void join(const char* a, const char* b, DynamicString& path)
	{
		const uint32_t la = strlen32(a);
		const uint32_t lb = strlen32(b);
		path.reserve(la + lb + 1);
		path += a;
		path += PATH_SEPARATOR;
		path += b;
	}

	const char* basename(const char* path)
	{
		const char* ls = strrchr(path, '/');
		return ls == NULL ? path : ls + 1;
	}

	const char* extension(const char* path)
	{
		const char* ld = strrchr(path, '.');
		return ld == NULL ? NULL : ld + 1;
	}
} // namespace path
} // namespace crown
