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

#include <algorithm>
#include <cstring>

#include "Assert.h"
#include "Allocator.h"
#include "StringUtils.h"
#include "List.h"

namespace crown
{

///
class DynamicString
{
public:

						DynamicString(Allocator& allocator = default_allocator());
						DynamicString(const char* s, Allocator& allocator = default_allocator());

						~DynamicString();

	DynamicString&		operator+=(DynamicString& s);
	DynamicString&		operator+=(const char* s);
	DynamicString&		operator+=(const char c);
	///
	DynamicString&		operator=(DynamicString& s);
	DynamicString&		operator=(const char* s);
	DynamicString&		operator=(const char c);

	bool				operator==(DynamicString& s);
	bool				operator==(const char* s);

	/// Removes the leading string @a s.
	/// @note
	/// The string must start with @a s.
	void				strip_leading(const char* s);

	/// Removes the trailing string @a s.
	/// @note
	/// The string must end with @a s.
	void				strip_trailing(const char* s);

	/// Returns whether the string starts with the given @a s string.
	bool				starts_with(const char* s);

	/// Returns wheterh the string ends with the given @æ s string.
	bool				ends_with(const char* s);

	///
	const char*			c_str();

private:

	List<char>			m_string;
};

//-----------------------------------------------------------------------------
inline DynamicString::DynamicString(Allocator& allocator)
	: m_string(allocator)
{
}

//-----------------------------------------------------------------------------
inline DynamicString::DynamicString(const char* s, Allocator& allocator)
	: m_string(allocator)
{
	if (s != NULL)
	{
		m_string.push(s, string::strlen(s));
	}
}

//-----------------------------------------------------------------------------
inline DynamicString::~DynamicString()
{
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator+=(DynamicString& s)
{
	const char* tmp = s.c_str();

	return *this += tmp;
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator+=(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	m_string.push(s, string::strlen(s));

	return *this;
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator+=(const char c)
{
	m_string.push_back(c);

	return *this;
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator=(DynamicString& s)
{
	const char* tmp = s.c_str();

	return *this = tmp;
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator=(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	m_string.clear();
	m_string.push(s, string::strlen(s));

	return *this;
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator=(const char c)
{
	m_string.clear();
	m_string.push_back(c);

	return *this;
}

//-----------------------------------------------------------------------------
inline bool DynamicString::operator==(DynamicString& s)
{
	return string::strcmp(c_str(), s.c_str()) == 0;
}

//-----------------------------------------------------------------------------
inline bool DynamicString::operator==(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	return string::strcmp(c_str(), s) == 0;
}

//-----------------------------------------------------------------------------
inline void DynamicString::strip_leading(const char* s)
{
	CE_ASSERT_NOT_NULL(s);
	CE_ASSERT(starts_with(s), "String does not start with %s", s);

	const size_t my_len = string::strlen(c_str());
	const size_t s_len = string::strlen(s);

	memmove(m_string.begin(), m_string.begin() + s_len, (my_len - s_len));
	m_string.resize(my_len - s_len);
}

//-----------------------------------------------------------------------------
inline void DynamicString::strip_trailing(const char* s)
{
	CE_ASSERT_NOT_NULL(s);
	CE_ASSERT(ends_with(s), "String does not end with %s", s);

	const size_t my_len = string::strlen(c_str());
	const size_t s_len = string::strlen(s);

	m_string.resize(my_len - s_len);
}

//-----------------------------------------------------------------------------
inline bool DynamicString::starts_with(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	return string::strncmp(c_str(), s, string::strlen(s)) == 0;
}

//-----------------------------------------------------------------------------
inline bool DynamicString::ends_with(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	const size_t my_len = string::strlen(c_str());
	const size_t s_len = string::strlen(s);

	if (my_len >= s_len)
	{
		return string::strncmp(m_string.begin() + (my_len - s_len), s, s_len) == 0;
	}

	return false;
}

//-----------------------------------------------------------------------------
inline const char* DynamicString::c_str()
{
	m_string.push_back('\0');
	m_string.pop_back();

	return m_string.begin();
}

} // namespace crown