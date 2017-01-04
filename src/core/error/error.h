/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"

/// @defgroup Error Error
/// @ingroup Core
namespace crown
{
/// Error management.
///
/// @ingroup Error
namespace error
{
	/// Aborts the program execution logging an error message and the stacktrace if
	/// the platform supports it.
	void abort(const char* file, int line, const char* format, ...);

	/// Prints the current call stack.
	void print_callstack();
} // namespace error

} // namespace crown

#if CROWN_DEBUG
	#define CE_ASSERT(condition, msg, ...)                  \
		do                                                  \
		{                                                   \
			if (!(condition))                               \
			{                                               \
				crown::error::abort(__FILE__                \
					, __LINE__                              \
					, "\nAssertion failed: %s\n\t" msg "\n" \
					, #condition                            \
					, ##__VA_ARGS__                         \
					);                                      \
			}                                               \
		} while (0)
#else
	#define CE_ASSERT(...) ((void)0)
#endif // CROWN_DEBUG

#define CE_FATAL(msg, ...) CE_ASSERT(false, msg, ##__VA_ARGS__)
#define CE_ENSURE(condition) CE_ASSERT(condition, "")
