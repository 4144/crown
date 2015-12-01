/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"

namespace crown
{
namespace error
{
	/// Aborts the program execution logging an error message and the stacktrace if
	/// the platform supports it.
	void abort(const char* file, int line, const char* message, ...);
} // namespace error
} // namespace crown

#if CROWN_DEBUG
	#define CE_ASSERT(condition, msg, ...) do { if (!(condition)) {\
		crown::error::abort(__FILE__, __LINE__, "\nAssertion failed: %s\n\t" msg "\n", #condition, ##__VA_ARGS__); }} while (0)
#else
	#define CE_ASSERT(...) ((void)0)
#endif // CROWN_DEBUG

#define CE_ASSERT_NOT_NULL(x) CE_ASSERT(x != NULL, #x " must be not null")
#define CE_FATAL(msg) CE_ASSERT(false, msg)
