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
#include "Vector.h"
#include "Map.h"
#include "Vector3.h"
#include "Quaternion.h"

namespace crown
{

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
	Array<const char*> array(default_allocator());

	json::parse_array(m_at, array);

	CE_ASSERT(i < array::size(array), "Index out of bounds");

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
		Array<const char*> array(default_allocator());

		json::parse_array(m_at, array);

		if (i >= array::size(array))
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
	Map<DynamicString, const char*> object(default_allocator());
	json::parse_object(m_at, object);

	const char* value = map::get(object, DynamicString(k), (const char*) NULL);
	CE_ASSERT(value != NULL, "Key not found: '%s'", k);

	return JSONElement(value);
}

//--------------------------------------------------------------------------
JSONElement JSONElement::key_or_nil(const char* k)
{
	if (m_at != NULL)
	{
		Map<DynamicString, const char*> object(default_allocator());
		json::parse_object(m_at, object);

		const char* value = map::get(object, DynamicString(k), (const char*) NULL);

		if (value)
			return JSONElement(value);
	}

	return JSONElement();
}

//--------------------------------------------------------------------------
bool JSONElement::has_key(const char* k) const
{
	Map<DynamicString, const char*> object(default_allocator());
	json::parse_object(m_at, object);

	return map::has(object, DynamicString(k));
}

//--------------------------------------------------------------------------
bool JSONElement::to_bool() const
{
	const bool value = json::parse_bool(m_at);
	return value;
}

//--------------------------------------------------------------------------
int32_t JSONElement::to_int() const
{
	const int32_t value = json::parse_int(m_at);
	return value;
}

//--------------------------------------------------------------------------
float JSONElement::to_float() const
{
	const float value = json::parse_float(m_at);
	return value;
}

//--------------------------------------------------------------------------
void JSONElement::to_string(DynamicString& str) const
{
	json::parse_string(m_at, str);
}

//--------------------------------------------------------------------------
Vector3 JSONElement::to_vector3() const
{
	TempAllocator64 alloc;
	Array<const char*> array(alloc);
	json::parse_array(m_at, array);

	return Vector3(json::parse_float(array[0]),
					json::parse_float(array[1]),
					json::parse_float(array[2]));
}

//--------------------------------------------------------------------------
Quaternion JSONElement::to_quaternion() const
{
	TempAllocator64 alloc;
	Array<const char*> array(alloc);
	json::parse_array(m_at, array);

	return Quaternion(json::parse_float(array[0]),
					json::parse_float(array[1]),
					json::parse_float(array[2]),
					json::parse_float(array[3]));
}

//--------------------------------------------------------------------------
StringId32 JSONElement::to_string_id() const
{
	TempAllocator1024 alloc;
	DynamicString str(alloc);
	json::parse_string(m_at, str);
	return str.to_string_id();
}

//--------------------------------------------------------------------------
void JSONElement::to_array(Array<bool>& array) const
{
	Array<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, json::parse_bool(temp[i]));
	}
}

//--------------------------------------------------------------------------
void JSONElement::to_array(Array<int16_t>& array) const
{
	Array<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, (int16_t)json::parse_int(temp[i]));
	}
}

//--------------------------------------------------------------------------
void JSONElement::to_array(Array<uint16_t>& array) const
{
	Array<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, (uint16_t)json::parse_int(temp[i]));
	}
}

//--------------------------------------------------------------------------
void JSONElement::to_array(Array<int32_t>& array) const
{
	Array<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, (int32_t)json::parse_int(temp[i]));
	}
}

//--------------------------------------------------------------------------
void JSONElement::to_array(Array<uint32_t>& array) const
{
	Array<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, (uint32_t)json::parse_int(temp[i]));
	}
}

//--------------------------------------------------------------------------
void JSONElement::to_array(Array<float>& array) const
{
	Array<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, json::parse_float(temp[i]));
	}
}

//--------------------------------------------------------------------------
void JSONElement::to_array(Vector<DynamicString>& array) const
{
	Array<const char*> temp(default_allocator());

	json::parse_array(m_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		DynamicString str;
		json::parse_string(temp[i], str);
		vector::push_back(array, str);
	}
}

//--------------------------------------------------------------------------
void JSONElement::to_keys(Vector<DynamicString>& keys) const
{
	Map<DynamicString, const char*> object(default_allocator());
	json::parse_object(m_at, object);

	const Map<DynamicString, const char*>::Node* it = map::begin(object);
	while (it != map::end(object))
	{
		vector::push_back(keys, (*it).key);
		it++;
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
			Map<DynamicString, const char*> object(default_allocator());
			json::parse_object(m_at, object);
			return map::size(object);
		}
		case JSONType::ARRAY:
		{
			Array<const char*> array(default_allocator());
			json::parse_array(m_at, array);

			return array::size(array);
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
