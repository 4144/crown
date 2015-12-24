/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "sjson.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "map.h"
#include "quaternion.h"

namespace crown
{
namespace sjson
{
	static const char* next(const char* json, const char c = 0)
	{
		CE_ASSERT_NOT_NULL(json);

		if (c && c != *json)
		{
			CE_ASSERT(false, "Expected '%c' got '%c'", c, *json);
		}

		return ++json;
	}

	static const char* skip_string(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		bool escaped = false;

		while ((*(json = next(json))) != 0)
		{
			if (*json == '"' && !escaped)
			{
				json = next(json);
				return json;
			}
			else if (*json == '\\') escaped = true;
			else escaped = false;
		}

		return json;
	}

	static const char* skip_value(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		switch (*json)
		{
			case '"': json = skip_string(json); break;
			case '[': json = skip_block(json, '[', ']'); break;
			case '{': json = skip_block(json, '{', '}'); break;
			default: for (; *json != '\0' && *json != ',' && *json != '\n' && *json != ' ' && *json != '}' && *json != ']'; ++json) ; break;
		}

		return json;
	}

	static const char* skip_comments(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		if (*json == '/')
		{
			json = next(json);
			if (*json == '/')
			{
				json = next(json, '/');
				while (*json && *json != '\n')
					json = next(json);
			}
			else if (*json == '*')
			{
				json = next(json);
				while (*json && *json != '*')
					json = next(json);
				json = next(json, '*');
				json = next(json, '/');
			}
			else
				CE_FATAL("Bad comment");
		}

		return json;
	}

	static const char* skip_spaces(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		while (*json)
		{
			if (*json == '/') json = skip_comments(json);
			else if (isspace(*json) || *json == ',') ++json;
			else break;
		}

		return json;
	}

	JsonValueType::Enum type(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		switch (*json)
		{
			case '"': return JsonValueType::STRING;
			case '{': return JsonValueType::OBJECT;
			case '[': return JsonValueType::ARRAY;
			case '-': return JsonValueType::NUMBER;
			default: return (isdigit(*json)) ? JsonValueType::NUMBER : (*json == 'n' ? JsonValueType::NIL : JsonValueType::BOOL);
		}
	}

	void parse_string(const char* json, DynamicString& string)
	{
		CE_ASSERT_NOT_NULL(json);

		if (*json == '"')
		{
			while (*(json = next(json)))
			{
				// Empty string
				if (*json == '"')
				{
					json = next(json);
					return;
				}
				else if (*json == '\\')
				{
					json = next(json);

					switch (*json)
					{
						case '"': string += '"'; break;
						case '\\': string += '\\'; break;
						case '/': string += '/'; break;
						case 'b': string += '\b'; break;
						case 'f': string += '\f'; break;
						case 'n': string += '\n'; break;
						case 'r': string += '\r'; break;
						case 't': string += '\t'; break;
						default:
						{
							CE_FATAL("Bad escape character");
							break;
						}
					}
				}
				else
				{
					string += *json;
				}
			}
		}

		CE_FATAL("Bad string");
	}

	static const char* parse_key(const char* json, DynamicString& key)
	{
		CE_ASSERT_NOT_NULL(json);
		if (*json == '"')
		{
			parse_string(json, key);
			return skip_string(json);
		}
		else if (isalpha(*json))
		{
			while (true)
			{
				if (isspace(*json) || *json == '=')
					return json;

				key += *json;
				++json;
			}
		}

		CE_FATAL("Bad key");
		return NULL;
	}

	double parse_number(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		TempAllocator512 alloc;
	 	Array<char> number(alloc);

		if (*json == '-')
		{
			array::push_back(number, '-');
			json = next(json, '-');
		}
		while (isdigit(*json))
		{
			array::push_back(number, *json);
			json = next(json);
		}

		if (*json == '.')
		{
			array::push_back(number, '.');
			while ((*(json = next(json))) && isdigit(*json))
			{
				array::push_back(number, *json);
			}
		}

		if (*json == 'e' || *json == 'E')
		{
			array::push_back(number, *json);
			json = next(json);

			if (*json == '-' || *json == '+')
			{
				array::push_back(number, *json);
				json = next(json);
			}
			while (isdigit(*json))
			{
				array::push_back(number, *json);
				json = next(json);
			}
		}

		// Ensure null terminated
		array::push_back(number, '\0');
		return parse_double(array::begin(number));
	}

	bool parse_bool(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		switch (*json)
		{
			case 't':
			{
				json = next(json, 't');
				json = next(json, 'r');
				json = next(json, 'u');
				json = next(json, 'e');
				return true;
			}
			case 'f':
			{
				json = next(json, 'f');
				json = next(json, 'a');
				json = next(json, 'l');
				json = next(json, 's');
				json = next(json, 'e');
				return false;
			}
			default:
			{
				CE_FATAL("Bad boolean");
				return false;
			}
		}
	}

	int32_t parse_int(const char* json)
	{
		return (int32_t) parse_number(json);
	}

	float parse_float(const char* json)
	{
		return (float) parse_number(json);
	}

	void parse_array(const char* json, JsonArray& array)
	{
		CE_ASSERT_NOT_NULL(json);

		if (*json == '[')
		{
			json = next(json, '[');
			json = skip_spaces(json);

			if (*json == ']')
			{
				json = next(json, ']');
				return;
			}

			while (*json)
			{
				array::push_back(array, json);

				json = skip_value(json);
				json = skip_spaces(json);

				if (*json == ']')
				{
					json = next(json, ']');
					return;
				}

				json = skip_spaces(json);
			}
		}

		CE_FATAL("Bad array");
	}

	void parse_root_object(const char* json, Map<DynamicString, const char*>& object)
	{
		CE_ASSERT_NOT_NULL(json);

		while (*json)
		{
			TempAllocator256 ta;
			DynamicString key(ta);
			json = parse_key(json, key);

			json = skip_spaces(json);
			json = next(json, '=');
			json = skip_spaces(json);

			map::set(object, key, json);

			json = skip_value(json);
			json = skip_spaces(json);
		}
	}

	void parse_object(const char* json, Map<DynamicString, const char*>& object)
	{
		CE_ASSERT_NOT_NULL(json);

		if (*json == '{')
		{
			json = next(json, '{');

			json = skip_spaces(json);

			if (*json == '}')
			{
				next(json, '}');
				return;
			}

			while (*json)
			{
				TempAllocator256 ta;
				DynamicString key(ta);
				json = parse_key(json, key);

				json = skip_spaces(json);
				json = next(json, '=');
				json = skip_spaces(json);

				map::set(object, key, json);

				json = skip_value(json);
				json = skip_spaces(json);

				if (*json == '}')
				{
					next(json, '}');
					return;
				}

				json = skip_spaces(json);
			}
		}

		CE_FATAL("Bad object");
	}

	void parse(const char* json, Map<DynamicString, const char*>& object)
	{
		CE_ASSERT_NOT_NULL(json);

		json = skip_spaces(json);

		if (*json == '{')
			parse_object(json, object);
		else
			parse_root_object(json, object);
	}

	void parse_root_object(const char* json, JsonObject& object)
	{
		CE_ASSERT_NOT_NULL(json);

		while (*json)
		{
			const char* key_begin = *json == '"' ? (json + 1) : json;

			TempAllocator256 ta;
			DynamicString key(ta);
			json = parse_key(json, key);

			FixedString fs_key(key_begin, key.length());

			json = skip_spaces(json);
			json = next(json, '=');
			json = skip_spaces(json);

			map::set(object, fs_key, json);

			json = skip_value(json);
			json = skip_spaces(json);
		}
	}

	void parse_object(const char* json, JsonObject& object)
	{
		CE_ASSERT_NOT_NULL(json);

		if (*json == '{')
		{
			json = next(json, '{');

			json = skip_spaces(json);

			if (*json == '}')
			{
				next(json, '}');
				return;
			}

			while (*json)
			{
				const char* key_begin = *json == '"' ? (json + 1) : json;

				TempAllocator256 ta;
				DynamicString key(ta);
				json = parse_key(json, key);

				FixedString fs_key(key_begin, key.length());

				json = skip_spaces(json);
				json = next(json, '=');
				json = skip_spaces(json);

				map::set(object, fs_key, json);

				json = skip_value(json);
				json = skip_spaces(json);

				if (*json == '}')
				{
					next(json, '}');
					return;
				}

				json = skip_spaces(json);
			}
		}

		CE_FATAL("Bad object");
	}

	void parse(const char* json, JsonObject& object)
	{
		CE_ASSERT_NOT_NULL(json);

		json = skip_spaces(json);

		if (*json == '{')
			parse_object(json, object);
		else
			parse_root_object(json, object);
	}

	void parse(Buffer& json, JsonObject& object)
	{
		array::push_back(json, '\0');
		array::pop_back(json);
		parse(array::begin(json), object);
	}
} // namespace sjson

namespace sjson
{
	Vector2 parse_vector2(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		sjson::parse_array(json, array);

		Vector2 v;
		v.x = sjson::parse_float(array[0]);
		v.y = sjson::parse_float(array[1]);
		return v;
	}

	Vector3 parse_vector3(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		sjson::parse_array(json, array);

		Vector3 v;
		v.x = sjson::parse_float(array[0]);
		v.y = sjson::parse_float(array[1]);
		v.z = sjson::parse_float(array[2]);
		return v;
	}

	Vector4 parse_vector4(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		sjson::parse_array(json, array);

		Vector4 v;
		v.x = sjson::parse_float(array[0]);
		v.y = sjson::parse_float(array[1]);
		v.z = sjson::parse_float(array[2]);
		v.w = sjson::parse_float(array[3]);
		return v;
	}

	Quaternion parse_quaternion(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		sjson::parse_array(json, array);

		Vector3 axis;
		axis.x = sjson::parse_float(array[0]);
		axis.y = sjson::parse_float(array[1]);
		axis.z = sjson::parse_float(array[2]);

		float angle = sjson::parse_float(array[3]);

		return quaternion(axis, angle);
	}

	Matrix4x4 parse_matrix4x4(const char* json)
	{
		TempAllocator128 ta;
		JsonArray array(ta);
		sjson::parse_array(json, array);

		Matrix4x4 m;
		m.x.x = sjson::parse_float(array[ 0]);
		m.x.y = sjson::parse_float(array[ 1]);
		m.x.z = sjson::parse_float(array[ 2]);
		m.x.w = sjson::parse_float(array[ 3]);

		m.y.x = sjson::parse_float(array[ 4]);
		m.y.y = sjson::parse_float(array[ 5]);
		m.y.z = sjson::parse_float(array[ 6]);
		m.y.w = sjson::parse_float(array[ 7]);

		m.z.x = sjson::parse_float(array[ 8]);
		m.z.y = sjson::parse_float(array[ 9]);
		m.z.z = sjson::parse_float(array[10]);
		m.z.w = sjson::parse_float(array[11]);

		m.t.x = sjson::parse_float(array[12]);
		m.t.y = sjson::parse_float(array[13]);
		m.t.z = sjson::parse_float(array[14]);
		m.t.w = sjson::parse_float(array[15]);
		return m;
	}

	StringId32 parse_string_id(const char* json)
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		sjson::parse_string(json, str);
		return str.to_string_id();
	}

	ResourceId parse_resource_id(const char* json)
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		sjson::parse_string(json, str);
		return ResourceId(str.c_str());
	}
} // namespace json

} // namespace crown
