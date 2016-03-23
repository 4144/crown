/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_BUILD_UNIT_TESTS

#include "array.h"
#include "command_line.h"
#include "dynamic_string.h"
#include "json.h"
#include "macros.h"
#include "math_utils.h"
#include "memory.h"
#include "murmur.h"
#include "path.h"
#include "sjson.h"
#include "string_id.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "vector.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

namespace crown
{
static void test_memory()
{
	memory_globals::init();
	Allocator& a = default_allocator();

	void* p = a.allocate(32);
	CE_ENSURE(a.allocated_size(p) >= 32);
	a.deallocate(p);

	memory_globals::shutdown();
}

static void test_array()
{
	memory_globals::init();
	Allocator& a = default_allocator();
	{
		Array<int> v(a);

		CE_ENSURE(array::size(v) == 0);
		array::push_back(v, 1);
		CE_ENSURE(array::size(v) == 1);
		CE_ENSURE(v[0] == 1);
	}
	memory_globals::shutdown();
}

static void test_vector()
{
	memory_globals::init();
	Allocator& a = default_allocator();
	{
		Vector<int> v(a);

		CE_ENSURE(vector::size(v) == 0);
		vector::push_back(v, 1);
		CE_ENSURE(vector::size(v) == 1);
		CE_ENSURE(v[0] == 1);
	}
	memory_globals::shutdown();
}

static void test_vector2()
{
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const Vector2 c = a - b;
		CE_ENSURE(fequal(c.x, -1.5f, 0.0001f));
		CE_ENSURE(fequal(c.y,  6.1f, 0.0001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const Vector2 c = a + b;
		CE_ENSURE(fequal(c.x,  3.9f, 0.0001f));
		CE_ENSURE(fequal(c.y,  2.3f, 0.0001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = a * 2.0f;
		CE_ENSURE(fequal(b.x,  2.4f, 0.0001f));
		CE_ENSURE(fequal(b.y,  8.4f, 0.0001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const f32 c = dot(a, b);
		CE_ENSURE(fequal(c,  -4.74f, 0.0001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const f32 c = length_squared(a);
		CE_ENSURE(fequal(c, 19.08f, 0.0001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const f32 c = length(a);
		CE_ENSURE(fequal(c,  4.36806f, 0.0001f));
	}
	{
		Vector2 a = vector2(1.2f,  4.2f);
		normalize(a);
		CE_ENSURE(fequal(length(a), 1.0f, 0.00001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const float c = distance_squared(a, b);
		CE_ENSURE(fequal(c, 39.46f, 0.00001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const float c = distance(a, b);
		CE_ENSURE(fequal(c, 6.28171f, 0.00001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const Vector2 c = max(a, b);
		CE_ENSURE(fequal(c.x,  2.7f, 0.00001f));
		CE_ENSURE(fequal(c.y,  4.2f, 0.00001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const Vector2 c = min(a, b);
		CE_ENSURE(fequal(c.x,  1.2f, 0.00001f));
		CE_ENSURE(fequal(c.y, -1.9f, 0.00001f));
	}
}

static void test_vector3()
{
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = a - b;
		CE_ENSURE(fequal(c.x, -1.5f, 0.0001f));
		CE_ENSURE(fequal(c.y,  6.1f, 0.0001f));
		CE_ENSURE(fequal(c.z,  1.8f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = a + b;
		CE_ENSURE(fequal(c.x,  3.9f, 0.0001f));
		CE_ENSURE(fequal(c.y,  2.3f, 0.0001f));
		CE_ENSURE(fequal(c.z, -6.4f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = a * 2.0f;
		CE_ENSURE(fequal(b.x,  2.4f, 0.0001f));
		CE_ENSURE(fequal(b.y,  8.4f, 0.0001f));
		CE_ENSURE(fequal(b.z, -4.6f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const f32 c = dot(a, b);
		CE_ENSURE(fequal(c,  4.69f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = cross(a, b);
		CE_ENSURE(fequal(c.x, -21.59f, 0.0001f));
		CE_ENSURE(fequal(c.y,  -1.29f, 0.0001f));
		CE_ENSURE(fequal(c.z, -13.62f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const f32 c = length_squared(a);
		CE_ENSURE(fequal(c, 24.37f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const f32 c = length(a);
		CE_ENSURE(fequal(c,  4.93659f, 0.0001f));
	}
	{
		Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		normalize(a);
		CE_ENSURE(fequal(length(a), 1.0f, 0.00001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const float c = distance_squared(a, b);
		CE_ENSURE(fequal(c, 42.70f, 0.00001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const float c = distance(a, b);
		CE_ENSURE(fequal(c, 6.53452f, 0.00001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = max(a, b);
		CE_ENSURE(fequal(c.x,  2.7f, 0.00001f));
		CE_ENSURE(fequal(c.y,  4.2f, 0.00001f));
		CE_ENSURE(fequal(c.z, -2.3f, 0.00001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = min(a, b);
		CE_ENSURE(fequal(c.x,  1.2f, 0.00001f));
		CE_ENSURE(fequal(c.y, -1.9f, 0.00001f));
		CE_ENSURE(fequal(c.z, -4.1f, 0.00001f));
	}
}

static void test_vector4()
{
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = a - b;
		CE_ENSURE(fequal(c.x, -1.5f, 0.0001f));
		CE_ENSURE(fequal(c.y,  6.1f, 0.0001f));
		CE_ENSURE(fequal(c.z,  1.8f, 0.0001f));
		CE_ENSURE(fequal(c.w,  4.5f, 0.0001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = a + b;
		CE_ENSURE(fequal(c.x,  3.9f, 0.0001f));
		CE_ENSURE(fequal(c.y,  2.3f, 0.0001f));
		CE_ENSURE(fequal(c.z, -6.4f, 0.0001f));
		CE_ENSURE(fequal(c.w,  6.5f, 0.0001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 1.5f);
		const Vector4 b = a * 2.0f;
		CE_ENSURE(fequal(b.x,  2.4f, 0.0001f));
		CE_ENSURE(fequal(b.y,  8.4f, 0.0001f));
		CE_ENSURE(fequal(b.z, -4.6f, 0.0001f));
		CE_ENSURE(fequal(b.w,  3.0f, 0.0001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const f32 c = dot(a, b);
		CE_ENSURE(fequal(c,  10.19f, 0.0001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const f32 c = length_squared(a);
		CE_ENSURE(fequal(c, 54.62f, 0.0001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const f32 c = length(a);
		CE_ENSURE(fequal(c,  7.39053f, 0.0001f));
	}
	{
		Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		normalize(a);
		CE_ENSURE(fequal(length(a), 1.0f, 0.00001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const float c = distance_squared(a, b);
		CE_ENSURE(fequal(c, 62.95f, 0.00001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const float c = distance(a, b);
		CE_ENSURE(fequal(c, 7.93410f, 0.00001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = max(a, b);
		CE_ENSURE(fequal(c.x,  2.7f, 0.00001f));
		CE_ENSURE(fequal(c.y,  4.2f, 0.00001f));
		CE_ENSURE(fequal(c.z, -2.3f, 0.00001f));
		CE_ENSURE(fequal(c.w,  5.5f, 0.00001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = min(a, b);
		CE_ENSURE(fequal(c.x,  1.2f, 0.00001f));
		CE_ENSURE(fequal(c.y, -1.9f, 0.00001f));
		CE_ENSURE(fequal(c.z, -4.1f, 0.00001f));
		CE_ENSURE(fequal(c.w,  1.0f, 0.00001f));
	}
}

static void test_murmur()
{
	const u32 m = murmur32("murmur32", 8, 0);
	CE_ENSURE(m == 0x7c2365dbu);
	const u64 n = murmur64("murmur64", 8, 0);
	CE_ENSURE(n == 0x90631502d1a3432bu);
}

static void test_string_id()
{
	{
		StringId32 a("murmur32");
		CE_ENSURE(a._id == 0x7c2365dbu);

		StringId32 b("murmur32", 8);
		CE_ENSURE(a._id == 0x7c2365dbu);

		char buf[128];
		a.to_string(buf);
		CE_ENSURE(strcmp(buf, "7c2365db") == 0);
	}
	{
		StringId64 a("murmur64");
		CE_ENSURE(a._id == 0x90631502d1a3432bu);

		StringId64 b("murmur64", 8);
		CE_ENSURE(a._id == 0x90631502d1a3432bu);

		char buf[128];
		a.to_string(buf);
		CE_ENSURE(strcmp(buf, "90631502d1a3432b") == 0);
	}
}

static void test_json()
{
	memory_globals::init();
	{
		JsonValueType::Enum t = json::type("null");
		CE_ENSURE(t == JsonValueType::NIL);
	}
	{
		JsonValueType::Enum t = json::type("true");
		CE_ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = json::type("false");
		CE_ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = json::type("3.14");
		CE_ENSURE(t == JsonValueType::NUMBER);
	}
	{
		JsonValueType::Enum t = json::type("\"foo\"");
		CE_ENSURE(t == JsonValueType::STRING);
	}
	{
		JsonValueType::Enum t = json::type("[]");
		CE_ENSURE(t == JsonValueType::ARRAY);
	}
	{
		JsonValueType::Enum t = json::type("{}");
		CE_ENSURE(t == JsonValueType::OBJECT);
	}
	{
		const s32 a = json::parse_int("3.14");
		CE_ENSURE(a == 3);
	}
	{
		const f32 a = json::parse_float("3.14");
		CE_ENSURE(fequal(a, 3.14f));
	}
	{
		const bool a = json::parse_bool("true");
		CE_ENSURE(a == true);
	}
	{
		const bool a = json::parse_bool("false");
		CE_ENSURE(a == false);
	}
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		json::parse_string("\"This is JSON\"", str);
		CE_ENSURE(strcmp(str.c_str(), "This is JSON") == 0);
	}
	memory_globals::shutdown();
}

static void test_sjson()
{
	memory_globals::init();
	{
		JsonValueType::Enum t = sjson::type("null");
		CE_ENSURE(t == JsonValueType::NIL);
	}
	{
		JsonValueType::Enum t = sjson::type("true");
		CE_ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = sjson::type("false");
		CE_ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = sjson::type("3.14");
		CE_ENSURE(t == JsonValueType::NUMBER);
	}
	{
		JsonValueType::Enum t = sjson::type("\"foo\"");
		CE_ENSURE(t == JsonValueType::STRING);
	}
	{
		JsonValueType::Enum t = sjson::type("[]");
		CE_ENSURE(t == JsonValueType::ARRAY);
	}
	{
		JsonValueType::Enum t = sjson::type("{}");
		CE_ENSURE(t == JsonValueType::OBJECT);
	}
	{
		const s32 a = sjson::parse_int("3.14");
		CE_ENSURE(a == 3);
	}
	{
		const f32 a = sjson::parse_float("3.14");
		CE_ENSURE(fequal(a, 3.14f));
	}
	{
		const bool a = sjson::parse_bool("true");
		CE_ENSURE(a == true);
	}
	{
		const bool a = sjson::parse_bool("false");
		CE_ENSURE(a == false);
	}
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		sjson::parse_string("\"This is JSON\"", str);
		CE_ENSURE(strcmp(str.c_str(), "This is JSON") == 0);
	}
	{
		const Vector2 a = sjson::parse_vector2("[ 1.2 -2.5 ]");
		CE_ENSURE(fequal(a.x,  1.2f));
		CE_ENSURE(fequal(a.y, -2.5f));
	}
	{
		const Vector3 a = sjson::parse_vector3("[ 3.1 0.5 -5.7]");
		CE_ENSURE(fequal(a.x,  3.1f));
		CE_ENSURE(fequal(a.y,  0.5f));
		CE_ENSURE(fequal(a.z, -5.7f));
	}
	{
		const Vector4 a = sjson::parse_vector4("[ 6.7 -1.3 2.9 -0.4 ]");
		CE_ENSURE(fequal(a.x,  6.7f));
		CE_ENSURE(fequal(a.y, -1.3f));
		CE_ENSURE(fequal(a.z,  2.9f));
		CE_ENSURE(fequal(a.w, -0.4f));
	}
	{
		const Quaternion a = sjson::parse_quaternion("[ -1.5 -3.4 9.1 -3.5 ]");
		CE_ENSURE(fequal(a.x, -1.5f));
		CE_ENSURE(fequal(a.y, -3.4f));
		CE_ENSURE(fequal(a.z,  9.1f));
		CE_ENSURE(fequal(a.w, -3.5f));
	}
	{
		const Matrix4x4 a = sjson::parse_matrix4x4(
			"["
			"-3.2  5.3 -0.7  4.1 "
			" 5.6  7.0 -3.2 -1.2 "
			"-6.3  9.0  3.9  1.1 "
			" 0.4 -7.3  8.9 -0.1 "
			"]"
			);
		CE_ENSURE(fequal(a.x.x, -3.2f));
		CE_ENSURE(fequal(a.x.y,  5.3f));
		CE_ENSURE(fequal(a.x.z, -0.7f));
		CE_ENSURE(fequal(a.x.w,  4.1f));
		CE_ENSURE(fequal(a.y.x,  5.6f));
		CE_ENSURE(fequal(a.y.y,  7.0f));
		CE_ENSURE(fequal(a.y.z, -3.2f));
		CE_ENSURE(fequal(a.y.w, -1.2f));
		CE_ENSURE(fequal(a.z.x, -6.3f));
		CE_ENSURE(fequal(a.z.y,  9.0f));
		CE_ENSURE(fequal(a.z.z,  3.9f));
		CE_ENSURE(fequal(a.z.w,  1.1f));
		CE_ENSURE(fequal(a.t.x,  0.4f));
		CE_ENSURE(fequal(a.t.y, -7.3f));
		CE_ENSURE(fequal(a.t.z,  8.9f));
		CE_ENSURE(fequal(a.t.w, -0.1f));
	}
	{
		const StringId32 a = sjson::parse_string_id("\"murmur32\"");
		CE_ENSURE(a._id == 0x7c2365dbu);
	}
	{
		const ResourceId a = sjson::parse_resource_id("\"murmur64\"");
		CE_ENSURE(a._id == 0x90631502d1a3432bu);
	}
	memory_globals::shutdown();
}

static void test_path()
{
#if CROWN_PLATFORM_POSIX
	{
		const bool a = path::is_absolute("/home/foo");
		CE_ENSURE(a == true);
		const bool b = path::is_absolute("home/foo");
		CE_ENSURE(b == false);
	}
	{
		const bool a = path::is_relative("/home/foo");
		CE_ENSURE(a == false);
		const bool b = path::is_relative("home/foo");
		CE_ENSURE(b == true);
	}
	{
		const bool a = path::is_root("/");
		CE_ENSURE(a == true);
		const bool b = path::is_root("/home");
		CE_ENSURE(b == false);
	}
#else
	{
		const bool a = path::is_absolute("C:\\Users\\foo");
		CE_ENSURE(a == true);
		const bool b = path::is_absolute("Users\\foo");
		CE_ENSURE(b == false);
	}
	{
		const bool a = path::is_relative("D:\\Users\\foo");
		CE_ENSURE(a == false);
		const bool b = path::is_relative("Users\\foo");
		CE_ENSURE(b == true);
	}
	{
		const bool a = path::is_root("E:\\");
		CE_ENSURE(a == true);
		const bool b = path::is_root("E:\\Users");
		CE_ENSURE(b == false);
	}
#endif // CROWN_PLATFORM_POSIX
	{
		const char* p = path::basename("");
		CE_ENSURE(strcmp(p, "") == 0);
		const char* q = path::basename("/");
		CE_ENSURE(strcmp(q, "") == 0);
		const char* r = path::basename("boot.config");
		CE_ENSURE(strcmp(r, "boot.config") == 0);
		const char* s = path::basename("foo/boot.config");
		CE_ENSURE(strcmp(s, "boot.config") == 0);
		const char* t = path::basename("/foo/boot.config");
		CE_ENSURE(strcmp(t, "boot.config") == 0);
	}
	{
		const char* p = path::extension("");
		CE_ENSURE(p == NULL);
		const char* q = path::extension("boot");
		CE_ENSURE(q == NULL);
		const char* r = path::extension("boot.bar.config");
		CE_ENSURE(strcmp(r, "config") == 0);
	}
}

static void test_command_line()
{
	const char* argv[] =
	{
		"args",
		"-s",
		"--switch",
		"--argument",
		"orange"
	};

	CommandLine cl(CE_COUNTOF(argv), argv);
	CE_ENSURE(cl.has_argument("switch", 's'));
	const char* orange = cl.get_parameter("argument");
	CE_ENSURE(orange != NULL && strcmp(orange, "orange") == 0);
}

static void run_unit_tests()
{
	test_memory();
	test_array();
	test_vector();
	test_vector2();
	test_vector3();
	test_vector4();
	test_murmur();
	test_string_id();
	test_json();
	test_sjson();
	test_path();
	test_command_line();
}

} // namespace crown

#endif // CROWN_BUILD_UNIT_TESTS
