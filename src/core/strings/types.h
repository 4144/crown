/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"

/// @defgroup String String
/// @ingroup Core
namespace crown
{
struct DynamicString;
struct FixedString;
struct StringId32;
struct StringId64;

typedef StringId64 ResourceId;

/// Stream of characters.
///
/// @ingroup String
typedef Array<char> StringStream;

} // namespace crown
