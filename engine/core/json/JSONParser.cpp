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

#include "JSONParser.h"
#include "JSON.h"
#include "TempAllocator.h"
#include "StringUtils.h"
#include "Log.h"

namespace crown
{

using json::JSONType;
using json::JSONPair;

//--------------------------------------------------------------------------
JSONElement::JSONElement()
	: m_at(NULL)
{
}

//--------------------------------------------------------------------------
JSONElement::JSONElement(const char* at)
	: m_at(at)
{
}

//--------------------------------------------------------------------------
JSONElement::JSONElement(const JSONElement& other)
	: m_at(other.m_at)
{
}

//--------------------------------------------------------------------------
JSONElement& JSONElement::operator=(const JSONElement& other)
{
	// Our begin is the other's at
	m_at = other.m_at;
	return *this;
}

//--------------------------------------------------------------------------
JSONElement JSONElement::operator[](uint32_t i)
{
	List<const char*> array(default_allocator());

	json::parse_array(m_at, array);

	CE_ASSERT(i < array.size(), "Index out of bounds");

	return JSONElement(array[i]);
}

//--------------------------------------------------------------------------
JSONElement JSONElement::index(uint32_t i)
{
	return this->operator[](i);
}

//--------------------------------------------------------------------------
JSONElement JSONElement::index_or_nil(uint32_t i)
{
	if (m_at != NULL)
	{
		List<const char*> array(default_allocator());

		json::parse_array(m_at, array);

		if (i >= array.size())
		{
			return JSONElement();
		}

		return JSONElement(array[i]);
	}

	return JSONElement();
}

//--------------------------------------------------------------------------
JSONElement JSONElement::key(const char* k)
{
	List<JSONPair> object(default_allocator());

	json::parse_object(m_at, object);

	bool found = false;

	const char* tmp_at = m_at;
	for (uint32_t i = 0; i < object.size(); i++)
	{
		DynamicString key;
		json::parse_string(object[i].key, key);

		if (key == k)
		{
			tmp_at = object[i].val;
			found = true;
		}
	}

	CE_ASSERT(found, "Key not found: '%s'", k);

	return JSONElement(tmp_at);
}

//--------------------------------------------------------------------------
JSONElement JSONElement::key_or_nil(const char* k)
{
	if (m_at != NULL)
	{
		List<JSONPair> object(default_allocator());

		json::parse_object(m_at, object);

		bool found = false;

		const char* tmp_at = m_at;
		for (uint32_t i = 0; i < object.size(); i++)
		{
			DynamicString key;
			json::parse_string(object[i].key, key);

			if (key == k)
			{
				tmp_at = object[i].val;
				found = true;
			}
		}

		if (!found)
		{
			return JSONElement();
		}

		return JSONElement(tmp_at);
	}

	return JSONElement();
}

//--------------------------------------------------------------------------
bool JSONElement::has_key(const char* k) const
{
	List<JSONPair> object(default_allocator());
	json::parse_object(m_at, object);

	for (uint32_t i = 0; i < object.size(); i++)
	{
		DynamicString key;
		json::parse_string(object[i].key, key);

		if (key == k)
		{
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------
bool JSONElement::is_key_unique(const char* k) const
{
	List<JSONPair> object(default_allocator());
	json::parse_object(m_at, object);

	bool found = false;

	for (uint32_t i = 0; i < object.size(); i++)
	{
		DynamicString key;
		json::parse_string(object[i].key, key);

		if (key == k)
		{
			if (found == true)
			{
				return false;
			}

			found = true;
		}
	}

	return found;
}

//--------------------------------------------------------------------------
bool JSONElement::bool_value() const
{
	const bool value = json::parse_bool(m_at);
	return value;
}

//--------------------------------------------------------------------------
int32_t JSONElement::int_value() const
{
	const int32_t value = json::parse_int(m_at);
	return value;
}

//--------------------------------------------------------------------------
float JSONElement::float_value() const
{
	const float value = json::parse_float(m_at);
	return value;
}

//--------------------------------------------------------------------------
void JSONElement::string_value(DynamicString& str) const
{
	json::parse_string(m_at, str);
}

//--------------------------------------------------------------------------
void JSONElement::array_value(List<bool>& array) const
{
	List<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < temp.size(); i++)
	{
		array.push_back(json::parse_bool(temp[i]));
	}
}

//--------------------------------------------------------------------------
void JSONElement::array_value(List<int16_t>& array) const
{
	List<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < temp.size(); i++)
	{
		array.push_back((int16_t)json::parse_int(temp[i]));
	}
}

//--------------------------------------------------------------------------
void JSONElement::array_value(List<uint16_t>& array) const
{
	List<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < temp.size(); i++)
	{
		array.push_back((uint16_t)json::parse_int(temp[i]));
	}
}

//--------------------------------------------------------------------------
void JSONElement::array_value(List<int32_t>& array) const
{
	List<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < temp.size(); i++)
	{
		array.push_back((int32_t)json::parse_int(temp[i]));
	}
}

//--------------------------------------------------------------------------
void JSONElement::array_value(List<uint32_t>& array) const
{
	List<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < temp.size(); i++)
	{
		array.push_back((uint32_t)json::parse_int(temp[i]));
	}
}

//--------------------------------------------------------------------------
void JSONElement::array_value(List<float>& array) const
{
	List<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < temp.size(); i++)
	{
		array.push_back(json::parse_float(temp[i]));
	}
}

//--------------------------------------------------------------------------
bool JSONElement::is_nil() const
{
	if (m_at != NULL)
	{
		return json::type(m_at) == JSONType::NIL;
	}

	return true;
}

//--------------------------------------------------------------------------
bool JSONElement::is_bool() const
{
	if (m_at != NULL)
	{
		return json::type(m_at) == JSONType::BOOL;
	}

	return false;
}

//--------------------------------------------------------------------------
bool JSONElement::is_number() const
{
	if (m_at != NULL)
	{
		return json::type(m_at) == JSONType::NUMBER;		
	}

	return false;
}

//--------------------------------------------------------------------------
bool JSONElement::is_string() const
{
	if (m_at != NULL)
	{
		return json::type(m_at) == JSONType::STRING;
	}

	return false;
}

//--------------------------------------------------------------------------
bool JSONElement::is_array() const
{
	if (m_at != NULL)
	{
		return json::type(m_at) == JSONType::ARRAY;
	}

	return false;
}

//--------------------------------------------------------------------------
bool JSONElement::is_object() const
{
	if (m_at != NULL)
	{
		return json::type(m_at) == JSONType::OBJECT;
	}

	return false;
}

//--------------------------------------------------------------------------
uint32_t JSONElement::size() const
{
	if (m_at == NULL)
	{
		return 0;
	}

	switch(json::type(m_at))
	{
		case JSONType::NIL:
		{
			return 1;
		}
		case JSONType::OBJECT:
		{
			List<JSONPair> object(default_allocator());
			json::parse_object(m_at, object);

			return object.size();
		}
		case JSONType::ARRAY:
		{
			List<const char*> array(default_allocator());
			json::parse_array(m_at, array);

			return array.size();
		}
		case JSONType::STRING:
		{
			DynamicString string;
			json::parse_string(m_at, string);
			return string.length();
		}
		case JSONType::NUMBER:
		{
			return 1;
		}
		case JSONType::BOOL:
		{
			return 1;
		}
		default:
		{
			CE_FATAL("Oops, unknown value type");
			return 0;
		}
	}
}

//--------------------------------------------------------------------------
JSONParser::JSONParser(const char* s)
	: m_document(s)
{
	CE_ASSERT_NOT_NULL(s);
}

//--------------------------------------------------------------------------
JSONElement JSONParser::root()
{
	const char* ch = m_document;
	while ((*ch) && (*ch) <= ' ') ch++;

	return JSONElement(ch);
}

} //namespace crown
