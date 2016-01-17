/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "color4.h"
#include "console_server.h"
#include "debug_line.h"
#include "device.h"
#include "gui.h"
#include "input_device.h"
#include "input_manager.h"
#include "intersection.h"
#include "lua_environment.h"
#include "lua_stack.h"
#include "math_types.h"
#include "math_utils.h"
#include "matrix4x4.h"
#include "physics_world.h"
#include "plane.h"
#include "profiler.h"
#include "quaternion.h"
#include "render_world.h"
#include "resource_manager.h"
#include "resource_package.h"
#include "scene_graph.h"
#include "sound_world.h"
#include "string_stream.h"
#include "temp_allocator.h"
#include "unit_manager.h"
#include "vector2.h"
#include "vector3.h"
#include "world.h"

namespace crown
{

struct LightInfo
{
	const char* name;
	LightType::Enum type;
};

static LightInfo s_light[] =
{
	{ "directional", LightType::DIRECTIONAL },
	{ "omni",        LightType::OMNI        },
	{ "spot",        LightType::SPOT        }
};
CE_STATIC_ASSERT(CE_COUNTOF(s_light) == LightType::COUNT);

struct ProjectionInfo
{
	const char* name;
	ProjectionType::Enum type;
};

static ProjectionInfo s_projection[] =
{
	{ "orthographic", ProjectionType::ORTHOGRAPHIC },
	{ "perspective",  ProjectionType::PERSPECTIVE  }
};
CE_STATIC_ASSERT(CE_COUNTOF(s_projection) == ProjectionType::COUNT);

struct RaycastInfo
{
	const char* name;
	RaycastMode::Enum mode;
};

static RaycastInfo s_raycast[] =
{
	{ "closest", RaycastMode::CLOSEST },
	{ "all",     RaycastMode::ALL     }
};
CE_STATIC_ASSERT(CE_COUNTOF(s_raycast) == RaycastMode::COUNT);

static LightType::Enum name_to_light_type(const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_light); ++i)
	{
		if (strcmp(s_light[i].name, name) == 0)
			return s_light[i].type;
	}

	return LightType::COUNT;
}

static ProjectionType::Enum name_to_projection_type(const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_projection); ++i)
	{
		if (strcmp(s_projection[i].name, name) == 0)
			return s_projection[i].type;
	}

	return ProjectionType::COUNT;
}

static RaycastMode::Enum name_to_raycast_mode(const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_raycast); ++i)
	{
		if (strcmp(s_raycast[i].name, name) == 0)
			return s_raycast[i].mode;
	}

	return RaycastMode::COUNT;
}

static int math_ray_plane_intersection(lua_State* L)
{
	LuaStack stack(L);
	const Plane p = plane::from_point_and_normal(stack.get_vector3(3)
		, stack.get_vector3(4)
		);
	const float t = ray_plane_intersection(stack.get_vector3(1)
		, stack.get_vector3(2)
		, p
		);
	stack.push_float(t);
	return 1;
}

static int math_ray_disc_intersection(lua_State* L)
{
	LuaStack stack(L);
	const float t = ray_disc_intersection(stack.get_vector3(1)
		, stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_float(4)
		, stack.get_vector3(5)
		);
	stack.push_float(t);
	return 1;
}

static int math_ray_sphere_intersection(lua_State* L)
{
	LuaStack stack(L);
	Sphere s;
	s.c = stack.get_vector3(3);
	s.r = stack.get_float(4);
	const float t = ray_sphere_intersection(stack.get_vector3(1)
		, stack.get_vector3(2)
		, s
		);
	stack.push_float(t);
	return 1;
}

static int math_ray_obb_intersection(lua_State* L)
{
	LuaStack stack(L);
	const float t = ray_obb_intersection(stack.get_vector3(1)
		, stack.get_vector3(2)
		, stack.get_matrix4x4(3)
		, stack.get_vector3(4)
		);
	stack.push_float(t);
	return 1;
}

static int vector3_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(vector3(stack.get_float(1), stack.get_float(2), stack.get_float(3)));
	return 1;
}

static int vector3_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return vector3_new(L);
}

static int vector3_x(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).x);
	return 1;
}

static int vector3_y(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).y);
	return 1;
}

static int vector3_z(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).z);
	return 1;
}

static int vector3_set_x(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).x = stack.get_float(2);
	return 0;
}

static int vector3_set_y(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).y = stack.get_float(2);
	return 0;
}

static int vector3_set_z(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).z = stack.get_float(2);
	return 0;
}

static int vector3_elements(lua_State* L)
{
	LuaStack stack(L);
	Vector3& a = stack.get_vector3(1);
	stack.push_float(a.x);
	stack.push_float(a.y);
	stack.push_float(a.z);
	return 3;
}

static int vector3_add(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) + stack.get_vector3(2));
	return 1;
}

static int vector3_subtract(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) - stack.get_vector3(2));
	return 1;
}

static int vector3_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) * stack.get_float(2));
	return 1;
}

static int vector3_dot(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(dot(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_cross(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(cross(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_equal(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_vector3(1) == stack.get_vector3(2));
	return 1;
}

static int vector3_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(length(stack.get_vector3(1)));
	return 1;
}

static int vector3_length_squared(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(length_squared(stack.get_vector3(1)));
	return 1;
}

static int vector3_set_length(lua_State* L)
{
	LuaStack stack(L);
	set_length(stack.get_vector3(1), stack.get_float(2));
	return 0;
}

static int vector3_normalize(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(normalize(stack.get_vector3(1)));
	return 1;
}

static int vector3_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(distance(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_distance_squared(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(distance_squared(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(angle(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_max(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(max(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_min(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(min(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_lerp(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(lerp(stack.get_vector3(1), stack.get_vector3(2), stack.get_float(3)));
	return 1;
}

static int vector3_forward(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_FORWARD);
	return 1;
}

static int vector3_backward(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_BACKWARD);
	return 1;
}

static int vector3_left(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_LEFT);
	return 1;
}

static int vector3_right(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_RIGHT);
	return 1;
}

static int vector3_up(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_UP);
	return 1;
}

static int vector3_down(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_DOWN);
	return 1;
}

static int vector3_zero(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_ZERO);
	return 1;
}

static int vector3_to_string(lua_State* L)
{
	LuaStack stack(L);
	const Vector3 v = stack.get_vector3(1);
	char buf[256];
	snprintf(buf, sizeof(buf), "%.4f %.4f %.4f", v.x, v.y, v.z);
	stack.push_string(buf);
	return 1;
}

static int vector2_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(vector2(stack.get_float(1), stack.get_float(2)));
	return 1;
}

static int vector2_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return vector2_new(L);
}

static int vector3box_new(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 0)
		stack.push_vector3box(VECTOR3_ZERO);
	else if (stack.num_args() == 1)
		stack.push_vector3box(stack.get_vector3(1));
	else
		stack.push_vector3box(vector3(stack.get_float(1)
			, stack.get_float(2)
			, stack.get_float(3)));

	return 1;
}

static int vector3box_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return vector3box_new(L);
}

static int vector3box_store(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);

	if (stack.num_args() == 2)
		v = stack.get_vector3(2);
	else
		v = vector3(stack.get_float(2)
			, stack.get_float(3)
			, stack.get_float(4));

	return 0;
}

static int vector3box_unbox(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3box(1));
	return 1;
}

static int vector3box_tostring(lua_State* L)
{
	LuaStack stack(L);
	Vector3& v = stack.get_vector3box(1);
	stack.push_fstring("Vector3Box (%p)", &v);
	return 1;
}

static int matrix4x4_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(stack.get_float(1)
		, stack.get_float(2)
		, stack.get_float(3)
		, stack.get_float(4)
		, stack.get_float(5)
		, stack.get_float(6)
		, stack.get_float(7)
		, stack.get_float(8)
		, stack.get_float(9)
		, stack.get_float(10)
		, stack.get_float(11)
		, stack.get_float(12)
		, stack.get_float(13)
		, stack.get_float(14)
		, stack.get_float(15)
		, stack.get_float(16)));
	return 1;
}

static int matrix4x4_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return matrix4x4_new(L);
}

static int matrix4x4_from_quaternion(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(stack.get_quaternion(1), vector3(0, 0, 0)));
	return 1;
}

static int matrix4x4_from_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(QUATERNION_IDENTITY, stack.get_vector3(1)));
	return 1;
}

static int matrix4x4_from_quaternion_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(stack.get_quaternion(1), stack.get_vector3(2)));
	return 1;
}

static int matrix4x4_from_axes(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(stack.get_vector3(1), stack.get_vector3(2), stack.get_vector3(3), stack.get_vector3(4)));
	return 1;
}

static int matrix4x4_copy(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4(1));
	return 1;
}

static int matrix4x4_add(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4(1) + stack.get_matrix4x4(2));
	return 1;
}

static int matrix4x4_subtract(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4(1) - stack.get_matrix4x4(2));
	return 1;
}

static int matrix4x4_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4(1) * stack.get_matrix4x4(2));
	return 1;
}

static int matrix4x4_transpose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(transpose(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_determinant(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(determinant(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_invert(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(invert(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_x(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(x(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_y(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(y(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_z(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(z(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_set_x(lua_State* L)
{
	LuaStack stack(L);
	set_x(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_set_y(lua_State* L)
{
	LuaStack stack(L);
	set_y(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_set_z(lua_State* L)
{
	LuaStack stack(L);
	set_z(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(translation(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_set_translation(lua_State* L)
{
	LuaStack stack(L);
	set_translation(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(rotation(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_set_rotation(lua_State* L)
{
	LuaStack stack(L);
	set_rotation(stack.get_matrix4x4(1), stack.get_quaternion(2));
	return 0;
}

static int matrix4x4_identity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(MATRIX4X4_IDENTITY);
	return 1;
}

static int matrix4x4_transform(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(2) * stack.get_matrix4x4(1));
	return 1;
}

static int matrix4x4_to_string(lua_State* L)
{
	LuaStack stack(L);
	Matrix4x4& a = stack.get_matrix4x4(1);
	char buf[1024];
	snprintf(buf, sizeof(buf),
		"%.4f, %.4f, %.4f, %.4f\n"
		"%.4f, %.4f, %.4f, %.4f\n"
		"%.4f, %.4f, %.4f, %.4f\n"
		"%.4f, %.4f, %.4f, %.4f"
		, a.x.x, a.x.y, a.x.z, a.y.w
		, a.y.x, a.y.y, a.y.z, a.y.w
		, a.z.x, a.z.y, a.z.z, a.z.w
		, a.t.x, a.t.y, a.t.z, a.t.w
		);
	stack.push_string(buf);
	return 1;
}

static int matrix4x4box_new(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 0)
		stack.push_matrix4x4(MATRIX4X4_IDENTITY);
	else
		stack.push_matrix4x4box(stack.get_matrix4x4(1));

	return 1;
}

static int matrix4x4box_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return matrix4x4box_new(L);
}

static int matrix4x4box_store(lua_State* L)
{
	LuaStack stack(L);
	stack.get_matrix4x4box(1) = stack.get_matrix4x4(2);
	return 0;
}

static int matrix4x4box_unbox(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4box(1));
	return 1;
}

static int matrix4x4box_tostring(lua_State* L)
{
	LuaStack stack(L);
	Matrix4x4& m = stack.get_matrix4x4box(1);
	stack.push_fstring("Matrix4x4Box (%p)", &m);
	return 1;
}

static int quaternion_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion(stack.get_vector3(1), stack.get_float(2)));
	return 1;
}

static int quaternion_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return quaternion_new(L);
}

static int quaternion_negate(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(-stack.get_quaternion(1));
	return 1;
}

static int quaternion_identity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(QUATERNION_IDENTITY);
	return 1;
}

static int quaternion_dot(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(dot(stack.get_quaternion(1), stack.get_quaternion(2)));
	return 1;
}

static int quaternion_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(length(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_normalize(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(normalize(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_conjugate(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(conjugate(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_inverse(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(inverse(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_quaternion(1) * stack.get_quaternion(2));
	return 1;
}

static int quaternion_multiply_by_scalar(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_quaternion(1) * stack.get_float(2));
	return 1;
}

static int quaternion_power(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(power(stack.get_quaternion(1), stack.get_float(2)));
	return 1;
}

static int quaternion_elements(lua_State* L)
{
	LuaStack stack(L);
	const Quaternion& q = stack.get_quaternion(1);
	stack.push_float(q.x);
	stack.push_float(q.y);
	stack.push_float(q.z);
	stack.push_float(q.w);
	return 4;
}

static int quaternion_look(lua_State* L)
{
	LuaStack stack(L);
	const Vector3 up = stack.num_args() == 2 ? stack.get_vector3(2) : VECTOR3_YAXIS;
	stack.push_quaternion(look(stack.get_vector3(1), up));
	return 1;
}

static int quaternion_right(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(right(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_up(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(up(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_forward(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(forward(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_lerp(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(lerp(stack.get_quaternion(1), stack.get_quaternion(2), stack.get_float(3)));
	return 1;
}

static int quaternion_to_string(lua_State* L)
{
	LuaStack stack(L);
	const Quaternion q = stack.get_quaternion(1);
	char buf[256];
	snprintf(buf, sizeof(buf), "%.4f %.4f %.4f %.4f", q.x, q.y, q.z, q.w);
	stack.push_string(buf);
	return 1;
}

static int quaternionbox_new(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 0)
		stack.push_quaternionbox(QUATERNION_IDENTITY);
	else if (stack.num_args() == 1)
		stack.push_quaternionbox(stack.get_quaternion(1));
	else
		stack.push_quaternionbox(quaternion(stack.get_float(1)
			, stack.get_float(2)
			, stack.get_float(3)
			, stack.get_float(4)));

	return 1;
}

static int quaternionbox_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return quaternionbox_new(L);
}

static int quaternionbox_store(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);

	if (stack.num_args() == 2)
		q = stack.get_quaternion(2);
	else
		q = quaternion(stack.get_float(2)
			, stack.get_float(3)
			, stack.get_float(4)
			, stack.get_float(5));

	return 0;
}

static int quaternionbox_unbox(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);

	stack.push_quaternion(q);
	return 1;
}

static int quaternionbox_tostring(lua_State* L)
{
	LuaStack stack(L);
	Quaternion& q = stack.get_quaternionbox(1);
	stack.push_fstring("QuaternionBox (%p)", &q);
	return 1;
}

static int color4_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion(stack.get_float(1)
		, stack.get_float(2)
		, stack.get_float(3)
		, stack.get_float(4)));
	return 1;
}

static int color4_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return color4_new(L);
}

static int lightuserdata_add(lua_State* L)
{
	LuaStack stack(L);
	const Vector3& a = stack.get_vector3(1);
	const Vector3& b = stack.get_vector3(2);
	stack.push_vector3(a + b);
	return 1;
}

static int lightuserdata_sub(lua_State* L)
{
	LuaStack stack(L);
	const Vector3& a = stack.get_vector3(1);
	const Vector3& b = stack.get_vector3(2);
	stack.push_vector3(a - b);
	return 1;
}

static int lightuserdata_mul(lua_State* L)
{
	LuaStack stack(L);
	const int i = stack.is_number(1) ? 1 : 2;
	stack.push_vector3(stack.get_float(i) * stack.get_vector3(3-i));
	return 1;
}

static int lightuserdata_unm(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(-stack.get_vector3(1));
	return 1;
}

static int lightuserdata_index(lua_State* L)
{
	LuaStack stack(L);
	Vector3& v = stack.get_vector3(1);
	const char* s = stack.get_string(2);

	switch (s[0])
	{
		case 'x': stack.push_float(v.x); return 1;
		case 'y': stack.push_float(v.y); return 1;
		case 'z': stack.push_float(v.z); return 1;
		default: LUA_ASSERT(false, stack, "Bad index: '%c'", s[0]); break;
	}

	return 0;
}

static int lightuserdata_newindex(lua_State* L)
{
	LuaStack stack(L);
	Vector3& v = stack.get_vector3(1);
	const char* s = stack.get_string(2);
	const float value = stack.get_float(3);

	switch (s[0])
	{
		case 'x': v.x = value; break;
		case 'y': v.y = value; break;
		case 'z': v.z = value; break;
		default: LUA_ASSERT(false, stack, "Bad index: '%c'", s[0]); break;
	}

	return 0;
}

static int input_device_name(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_string(id.name());
	return 1;
}

static int input_device_connected(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.connected());
	return 1;
}

static int input_device_num_buttons(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_int(id.num_buttons());
	return 1;
}

static int input_device_num_axes(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_int(id.num_axes());
	return 1;
}

static int input_device_pressed(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.pressed(stack.get_int(1)));
	return 1;
}

static int input_device_released(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.released(stack.get_int(1)));
	return 1;
}

static int input_device_any_pressed(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.any_pressed());
	return 1;
}

static int input_device_any_released(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.any_released());
	return 1;
}

static int input_device_axis(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_vector3(id.axis(stack.get_int(1)));
	return 1;
}

static int input_device_button_name(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_string(id.button_name(stack.get_int(1)));
	return 1;
}

static int input_device_axis_name(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_string(id.axis_name(stack.get_int(1)));
	return 1;
}

static int input_device_button_id(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_int(id.button_id(stack.get_string_id_32(1)));
	return 1;
}

static int input_device_axis_id(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_int(id.axis_id(stack.get_string_id_32(1)));
	return 1;
}

#define KEYBOARD_FN(name) keyboard_##name
#define MOUSE_FN(name) mouse_##name
#define TOUCH_FN(name) touch_##name
#define JOYPAD_FN(index, name) joypad_##name##index

#define KEYBOARD(name) static int KEYBOARD_FN(name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->keyboard()); }
#define MOUSE(name) static int MOUSE_FN(name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->mouse()); }
#define TOUCH(name) static int TOUCH_FN(name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->touch()); }
#define JOYPAD(index, name) static int JOYPAD_FN(index, name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->joypad(index)); }

KEYBOARD(name)
KEYBOARD(connected)
KEYBOARD(num_buttons)
KEYBOARD(num_axes)
KEYBOARD(pressed)
KEYBOARD(released)
KEYBOARD(any_pressed)
KEYBOARD(any_released)
// KEYBOARD(axis)
KEYBOARD(button_name)
// KEYBOARD(axis_name)
KEYBOARD(button_id)
// KEYBOARD(axis_id)

MOUSE(name)
MOUSE(connected)
MOUSE(num_buttons)
MOUSE(num_axes)
MOUSE(pressed)
MOUSE(released)
MOUSE(any_pressed)
MOUSE(any_released)
MOUSE(axis)
MOUSE(button_name)
MOUSE(axis_name)
MOUSE(button_id)
MOUSE(axis_id)

TOUCH(name)
TOUCH(connected)
TOUCH(num_buttons)
TOUCH(num_axes)
TOUCH(pressed)
TOUCH(released)
TOUCH(any_pressed)
TOUCH(any_released)
TOUCH(axis)
TOUCH(button_name)
TOUCH(axis_name)
TOUCH(button_id)
TOUCH(axis_id)

JOYPAD(0, name)
JOYPAD(0, connected)
JOYPAD(0, num_buttons)
JOYPAD(0, num_axes)
JOYPAD(0, pressed)
JOYPAD(0, released)
JOYPAD(0, any_pressed)
JOYPAD(0, any_released)
JOYPAD(0, axis)
JOYPAD(0, button_name)
JOYPAD(0, axis_name)
JOYPAD(0, button_id)
JOYPAD(0, axis_id)

JOYPAD(1, name)
JOYPAD(1, connected)
JOYPAD(1, num_buttons)
JOYPAD(1, num_axes)
JOYPAD(1, pressed)
JOYPAD(1, released)
JOYPAD(1, any_pressed)
JOYPAD(1, any_released)
JOYPAD(1, axis)
JOYPAD(1, button_name)
JOYPAD(1, axis_name)
JOYPAD(1, button_id)
JOYPAD(1, axis_id)

JOYPAD(2, name)
JOYPAD(2, connected)
JOYPAD(2, num_buttons)
JOYPAD(2, num_axes)
JOYPAD(2, pressed)
JOYPAD(2, released)
JOYPAD(2, any_pressed)
JOYPAD(2, any_released)
JOYPAD(2, axis)
JOYPAD(2, button_name)
JOYPAD(2, axis_name)
JOYPAD(2, button_id)
JOYPAD(2, axis_id)

JOYPAD(3, name)
JOYPAD(3, connected)
JOYPAD(3, num_buttons)
JOYPAD(3, num_axes)
JOYPAD(3, pressed)
JOYPAD(3, released)
JOYPAD(3, any_pressed)
JOYPAD(3, any_released)
JOYPAD(3, axis)
JOYPAD(3, button_name)
JOYPAD(3, axis_name)
JOYPAD(3, button_id)
JOYPAD(3, axis_id)

static int world_spawn_unit(lua_State* L)
{
	LuaStack stack(L);
	World* world = stack.get_world(1);
	const StringId64 name = stack.get_resource_id(2);
	const Vector3& pos = stack.num_args() > 2 ? stack.get_vector3(3) : VECTOR3_ZERO;
	const Quaternion& rot = stack.num_args() > 3 ? stack.get_quaternion(4) : QUATERNION_IDENTITY;

	LUA_ASSERT(device()->resource_manager()->can_get(UNIT_TYPE, name), stack, "Unit not found");

	UnitId unit = world->spawn_unit(name, pos, rot);
	stack.push_unit(unit);
	return 1;
}

static int world_destroy_unit(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->destroy_unit(stack.get_unit(2));
	return 0;
}

static int world_num_units(lua_State* L)
{
	LuaStack stack(L);
	stack.push_int(stack.get_world(1)->num_units());
	return 1;
}

static int world_units(lua_State* L)
{
	LuaStack stack(L);

	TempAllocator1024 alloc;
	Array<UnitId> units(alloc);
	stack.get_world(1)->units(units);

	const uint32_t num = array::size(units);

	stack.push_table(num);
	for (uint32_t i = 0; i < num; ++i)
	{
		stack.push_key_begin((int32_t) i + 1);
		stack.push_unit(units[i]);
		stack.push_key_end();
	}

	return 1;
}

static int world_camera(lua_State* L)
{
	LuaStack stack(L);
	stack.push_camera(stack.get_world(1)->camera(stack.get_unit(2)));
	return 1;
}

static int camera_set_projection_type(lua_State* L)
{
	LuaStack stack(L);

	const char* name = stack.get_string(3);
	const ProjectionType::Enum pt = name_to_projection_type(name);
	LUA_ASSERT(pt != ProjectionType::COUNT, stack, "Unknown projection type: '%s'", name);

	stack.get_world(1)->set_camera_projection_type(stack.get_camera(2), pt);
	return 0;
}

static int camera_projection_type(lua_State* L)
{
	LuaStack stack(L);
	ProjectionType::Enum type = stack.get_world(1)->camera_projection_type(stack.get_camera(2));
	stack.push_string(s_projection[type].name);
	return 1;
}

static int camera_fov(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_world(1)->camera_fov(stack.get_camera(2)));
	return 1;
}

static int camera_set_fov(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_fov(stack.get_camera(2), stack.get_float(3));
	return 0;
}

static int camera_aspect(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_world(1)->camera_aspect(stack.get_camera(2)));
	return 1;
}

static int camera_set_aspect(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_aspect(stack.get_camera(2), stack.get_float(3));
	return 0;
}

static int camera_near_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_world(1)->camera_near_clip_distance(stack.get_camera(2)));
	return 1;
}

static int camera_set_near_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_near_clip_distance(stack.get_camera(2), stack.get_float(3));
	return 0;
}

static int camera_far_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_world(1)->camera_far_clip_distance(stack.get_camera(2)));
	return 1;
}

static int camera_set_far_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_far_clip_distance(stack.get_camera(2), stack.get_float(3));
	return 0;
}

static int camera_set_orthographic_metrics(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_orthographic_metrics(stack.get_camera(2), stack.get_float(3), stack.get_float(4),
		stack.get_float(5), stack.get_float(6));
	return 0;
}

static int camera_set_viewport_metrics(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_viewport_metrics(stack.get_camera(2), stack.get_int(3), stack.get_int(4),
		stack.get_int(5), stack.get_int(6));
	return 0;
}

static int camera_screen_to_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_world(1)->camera_screen_to_world(stack.get_camera(2), stack.get_vector3(3)));
	return 1;
}

static int camera_world_to_screen(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_world(1)->camera_world_to_screen(stack.get_camera(2), stack.get_vector3(3)));
	return 1;
}

static int world_update_animations(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update_animations(stack.get_float(2));
	return 0;
}

static int world_update_scene(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update_scene(stack.get_float(2));
	return 0;
}

static int world_update(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update(stack.get_float(2));
	return 0;
}

static int world_play_sound(lua_State* L)
{
	LuaStack stack(L);
	World* world = stack.get_world(1);
	const StringId64 name = stack.get_resource_id(2);
	const int32_t nargs = stack.num_args();
	const bool loop = nargs > 2 ? stack.get_bool(3) : false;
	const float volume = nargs > 3 ? stack.get_float(4) : 1.0f;
	const Vector3& pos = nargs > 4 ? stack.get_vector3(5) : VECTOR3_ZERO;
	const float range = nargs > 5 ? stack.get_float(6) : 1000.0f;

	LUA_ASSERT(device()->resource_manager()->can_get(SOUND_TYPE, name), stack, "Sound not found");

	stack.push_sound_instance_id(world->play_sound(name, loop, volume, pos, range));
	return 1;
}

static int world_stop_sound(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->stop_sound(stack.get_sound_instance_id(2));
	return 0;
}

static int world_link_sound(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->link_sound(stack.get_sound_instance_id(2)
		, stack.get_unit(3)
		, stack.get_int(4)
		);
	return 0;
}

static int world_set_listener_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_listener_pose(stack.get_matrix4x4(2));
	return 0;
}

static int world_set_sound_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_position(stack.get_sound_instance_id(2), stack.get_vector3(3));
	return 0;
}

static int world_set_sound_range(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_range(stack.get_sound_instance_id(2),
		stack.get_float(3));
	return 0;
}

static int world_set_sound_volume(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_volume(stack.get_sound_instance_id(2), stack.get_float(3));
	return 0;
}

static int world_create_debug_line(lua_State* L)
{
	LuaStack stack(L);
	stack.push_debug_line(stack.get_world(1)->create_debug_line(stack.get_bool(2)));
	return 1;
}

static int world_destroy_debug_line(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->destroy_debug_line(*stack.get_debug_line(2));
	return 0;
}

static int world_load_level(lua_State* L)
{
	LuaStack stack(L);
	const StringId64 name = stack.get_resource_id(2);
	const Vector3& pos = stack.num_args() > 2 ? stack.get_vector3(3) : VECTOR3_ZERO;
	const Quaternion& rot = stack.num_args() > 3 ? stack.get_quaternion(4) : QUATERNION_IDENTITY;
	LUA_ASSERT(device()->resource_manager()->can_get(LEVEL_TYPE, name), stack, "Level not found");
	stack.push_level(stack.get_world(1)->load_level(name, pos, rot));
	return 1;
}

static int world_scene_graph(lua_State* L)
{
	LuaStack stack(L);
	stack.push_scene_graph(stack.get_world(1)->scene_graph());
	return 1;
}

static int world_render_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_render_world(stack.get_world(1)->render_world());
	return 1;
}

static int world_physics_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_physics_world(stack.get_world(1)->physics_world());
	return 1;
}

static int world_sound_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_sound_world(stack.get_world(1)->sound_world());
	return 1;
}

static int world_tostring(lua_State* L)
{
	LuaStack stack(L);
	World* w = stack.get_world(1);
	stack.push_fstring("World (%p)", w);
	return 1;
}

static int scene_graph_create(lua_State* L)
{
	LuaStack stack(L);
	stack.push_transform(stack.get_scene_graph(1)->create(stack.get_unit(2), MATRIX4X4_IDENTITY));
	return 1;
}

static int scene_graph_destroy(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->destroy(stack.get_transform(2));
	return 0;
}

static int scene_graph_transform_instances(lua_State* L)
{
	LuaStack stack(L);
	stack.push_transform(stack.get_scene_graph(1)->get(stack.get_unit(2)));
	return 1;
}

static int scene_graph_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_scene_graph(1)->local_position(stack.get_transform(2)));
	return 1;
}

static int scene_graph_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_scene_graph(1)->local_rotation(stack.get_transform(2)));
	return 1;
}

static int scene_graph_local_scale(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_scene_graph(1)->local_scale(stack.get_transform(2)));
	return 1;
}

static int scene_graph_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_scene_graph(1)->local_pose(stack.get_transform(2)));
	return 1;
}

static int scene_graph_world_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_scene_graph(1)->world_position(stack.get_transform(2)));
	return 1;
}

static int scene_graph_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_scene_graph(1)->world_rotation(stack.get_transform(2)));
	return 1;
}

static int scene_graph_world_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_scene_graph(1)->world_pose(stack.get_transform(2)));
	return 1;
}

static int scene_graph_set_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->set_local_position(stack.get_transform(2), stack.get_vector3(3));
	return 0;
}

static int scene_graph_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->set_local_rotation(stack.get_transform(2), stack.get_quaternion(3));
	return 0;
}

static int scene_graph_set_local_scale(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->set_local_scale(stack.get_transform(2), stack.get_vector3(3));
	return 0;
}

static int scene_graph_set_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->set_local_pose(stack.get_transform(2), stack.get_matrix4x4(3));
	return 0;
}

static int scene_graph_link(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->link(stack.get_transform(2), stack.get_transform(3));
	return 0;
}

static int scene_graph_unlink(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->unlink(stack.get_transform(2));
	return 0;
}

static int unit_manager_create(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 1)
		stack.push_unit(device()->unit_manager()->create(*stack.get_world(1)));
	else
		stack.push_unit(device()->unit_manager()->create());

	return 1;
}

static int unit_manager_alive(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->unit_manager()->alive(stack.get_unit(1)));
	return 1;
}

static int render_world_create_mesh(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	UnitId unit = stack.get_unit(2);

	MeshRendererDesc desc;
	desc.mesh_resource = stack.get_resource_id(3);
	desc.mesh_name = stack.get_string_id_32(4);
	desc.material_resource = stack.get_resource_id(5);
	desc.visible = stack.get_bool(6);

	stack.push_mesh_instance(rw->create_mesh(unit, desc, MATRIX4X4_IDENTITY));
	return 1;
}

static int render_world_destroy_mesh(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->destroy_mesh(stack.get_mesh_instance(2));
	return 0;
}

static int render_world_mesh_instances(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	UnitId unit = stack.get_unit(2);
	MeshInstance inst = rw->first_mesh(unit);

	stack.push_table();
	for (uint32_t i = 0; rw->is_valid(inst); ++i, inst = rw->next_mesh(inst))
	{
		stack.push_key_begin(i+1);
		stack.push_mesh_instance(inst);
		stack.push_key_end();
	}

	return 1;
}

static int render_world_mesh_obb(lua_State* L)
{
	LuaStack stack(L);
	OBB obb = stack.get_render_world(1)->mesh_obb(stack.get_mesh_instance(2));
	stack.push_matrix4x4(obb.tm);
	stack.push_vector3(obb.half_extents);
	return 2;
}

static int render_world_set_mesh_visible(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_mesh_visible(stack.get_mesh_instance(2), stack.get_bool(3));
	return 0;
}

static int render_world_create_sprite(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	UnitId unit = stack.get_unit(2);

	SpriteRendererDesc desc;
	desc.sprite_resource = stack.get_resource_id(3);
	desc.material_resource = stack.get_resource_id(4);
	desc.visible = stack.get_bool(5);

	stack.push_sprite_instance(rw->create_sprite(unit, desc, MATRIX4X4_IDENTITY));
	return 1;
}

static int render_world_destroy_sprite(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->destroy_sprite(stack.get_sprite_instance(2));
	return 0;
}

static int render_world_sprite_instances(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	UnitId unit = stack.get_unit(2);
	SpriteInstance inst = rw->first_sprite(unit);

	stack.push_table();
	for (uint32_t i = 0; rw->is_valid(inst); ++i, inst = rw->next_sprite(inst))
	{
		stack.push_key_begin(i+1);
		stack.push_sprite_instance(inst);
		stack.push_key_end();
	}

	return 1;
}

static int render_world_set_sprite_visible(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_sprite_visible(stack.get_sprite_instance(2), stack.get_bool(3));
	return 0;
}

static int render_world_set_sprite_frame(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_sprite_frame(stack.get_sprite_instance(2), stack.get_int(3));
	return 0;
}

static int render_world_create_light(lua_State* L)
{
	LuaStack stack(L);
	LightDesc ld;
	ld.type = LightType::DIRECTIONAL;
	ld.range = 1.0f;
	ld.intensity = 1.0f;
	ld.spot_angle = 20.0f;
	ld.color = vector3(1, 1, 1);
	stack.push_light_instance(stack.get_render_world(1)->create_light(stack.get_unit(2), ld, MATRIX4X4_IDENTITY));
	return 1;
}

static int render_world_destroy_light(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->destroy_light(stack.get_light_instance(2));
	return 0;
}

static int render_world_light_instances(lua_State* L)
{
	LuaStack stack(L);
	stack.push_light_instance(stack.get_render_world(1)->light(stack.get_unit(2)));
	return 1;
}

static int render_world_light_type(lua_State* L)
{
	LuaStack stack(L);
	LightType::Enum type = stack.get_render_world(1)->light_type(stack.get_light_instance(2));
	stack.push_string(s_light[type].name);
	return 1;
}

static int render_world_light_color(lua_State* L)
{
	LuaStack stack(L);
	stack.push_color4(stack.get_render_world(1)->light_color(stack.get_light_instance(2)));
	return 1;
}

static int render_world_light_range(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_render_world(1)->light_range(stack.get_light_instance(2)));
	return 1;
}

static int render_world_light_intensity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_render_world(1)->light_intensity(stack.get_light_instance(2)));
	return 1;
}

static int render_world_light_spot_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_render_world(1)->light_spot_angle(stack.get_light_instance(2)));
	return 1;
}

static int render_world_set_light_type(lua_State* L)
{
	LuaStack stack(L);

	const char* name = stack.get_string(3);
	const LightType::Enum lt = name_to_light_type(name);
	LUA_ASSERT(lt != LightType::COUNT, stack, "Unknown light type: '%s'", name);

	stack.get_render_world(1)->set_light_type(stack.get_light_instance(2), lt);
	return 0;
}

static int render_world_set_light_color(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_light_color(stack.get_light_instance(2), stack.get_color4(3));
	return 0;
}

static int render_world_set_light_range(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_light_range(stack.get_light_instance(2), stack.get_float(3));
	return 0;
}

static int render_world_set_light_intensity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_light_intensity(stack.get_light_instance(2), stack.get_float(3));
	return 0;
}

static int render_world_set_light_spot_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_light_spot_angle(stack.get_light_instance(2), stack.get_float(3));
	return 0;
}

static int render_world_enable_debug_drawing(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->enable_debug_drawing(stack.get_bool(2));
	return 0;
}

static int physics_world_actor_instances(lua_State* L)
{
	LuaStack stack(L);
	ActorInstance inst = stack.get_physics_world(1)->actor(stack.get_unit(2));

	if (inst.i == UINT32_MAX)
		stack.push_nil();
	else
		stack.push_actor(inst);

	return 1;
}

static int physics_world_actor_world_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_world_position(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_physics_world(1)->actor_world_rotation(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_world_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_physics_world(1)->actor_world_pose(stack.get_actor(2)));
	return 1;
}

static int physics_world_teleport_actor_world_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->teleport_actor_world_position(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_teleport_actor_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->teleport_actor_world_rotation(stack.get_actor(2), stack.get_quaternion(3));
	return 0;
}

static int physics_world_teleport_actor_world_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->teleport_actor_world_pose(stack.get_actor(2), stack.get_matrix4x4(3));
	return 0;
}

static int physics_world_actor_center_of_mass(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_center_of_mass(stack.get_actor(2)));
	return 1;
}

static int physics_world_enable_actor_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->enable_actor_gravity(stack.get_actor(2));
	return 0;
}

static int physics_world_disable_actor_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->disable_actor_gravity(stack.get_actor(2));
	return 0;
}

static int physics_world_enable_actor_collision(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->enable_actor_collision(stack.get_actor(2));
	return 0;
}

static int physics_world_disable_actor_collision(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->disable_actor_collision(stack.get_actor(2));
	return 0;
}

static int physics_world_set_actor_collision_filter(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_collision_filter(stack.get_actor(2), stack.get_string_id_32(3));
	return 0;
}

static int physics_world_set_actor_kinematic(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_kinematic(stack.get_actor(2), stack.get_bool(3));
	return 0;
}

static int physics_world_move_actor(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->move_actor(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_is_static(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->is_static(stack.get_actor(2)));
	return 1;
}

static int physics_world_is_dynamic(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->is_dynamic(stack.get_actor(2)));
	return 1;
}

static int physics_world_is_kinematic(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->is_kinematic(stack.get_actor(2)));
	return 1;
}

static int physics_world_is_nonkinematic(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->is_nonkinematic(stack.get_actor(2)));
	return 1;
}


static int physics_world_actor_linear_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_physics_world(1)->actor_linear_damping(stack.get_actor(2)));
	return 1;
}

static int physics_world_set_actor_linear_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_linear_damping(stack.get_actor(2), stack.get_float(3));
	return 0;
}

static int physics_world_actor_angular_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_physics_world(1)->actor_angular_damping(stack.get_actor(2)));
	return 1;
}

static int physics_world_set_actor_angular_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_angular_damping(stack.get_actor(2), stack.get_float(3));
	return 0;
}

static int physics_world_actor_linear_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_linear_velocity(stack.get_actor(2)));
	return 1;
}

static int physics_world_set_actor_linear_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_linear_velocity(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_actor_angular_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_angular_velocity(stack.get_actor(2)));
	return 1;
}

static int physics_world_set_actor_angular_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_angular_velocity(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_add_actor_impulse(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->add_actor_impulse(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_add_actor_impulse_at(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->add_actor_impulse_at(stack.get_actor(2), stack.get_vector3(3), stack.get_vector3(4));
	return 0;
}

static int physics_world_add_actor_torque_impulse(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->add_actor_torque_impulse(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_push_actor(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->push_actor(stack.get_actor(2), stack.get_vector3(3), stack.get_float(4));
	return 0;
}

static int physics_world_push_actor_at(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->push_actor_at(stack.get_actor(2), stack.get_vector3(3), stack.get_float(4), stack.get_vector3(5));
	return 0;
}

static int physics_world_is_sleeping(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->is_sleeping(stack.get_actor(2)));
	return 1;
}

static int physics_world_wake_up(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->wake_up(stack.get_actor(2));
	return 0;
}

static int physics_world_controller_instances(lua_State* L)
{
	LuaStack stack(L);
	stack.push_controller(stack.get_physics_world(1)->controller(stack.get_unit(2)));
	return 1;
}

static int physics_world_move_controller(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->move_controller(stack.get_controller(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_create_joint(lua_State* L)
{
	LuaStack stack(L);
	JointDesc jd;
	jd.type = JointType::SPRING;
	jd.anchor_0 = vector3(0, -2, 0);
	jd.anchor_1 = vector3(0, 2, 0);
	jd.break_force = 999999.0f;
	jd.hinge.axis = vector3(1, 0, 0);
	jd.hinge.lower_limit = -3.14f / 4.0f;
	jd.hinge.upper_limit = 3.14f / 4.0f;
	jd.hinge.bounciness = 12.0f;
	stack.get_physics_world(1)->create_joint(stack.get_actor(2), stack.get_actor(3), jd);
	return 0;
}

static int physics_world_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->gravity());
	return 1;
}

static int physics_world_set_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_gravity(stack.get_vector3(2));
	return 0;
}

static int physics_world_raycast(lua_State* L)
{
	LuaStack stack(L);
	PhysicsWorld* world = stack.get_physics_world(1);

	const char* name = stack.get_string(5);
	const RaycastMode::Enum mode = name_to_raycast_mode(name);
	LUA_ASSERT(mode != RaycastMode::COUNT, stack, "Unknown raycast mode: '%s'", name);

	TempAllocator1024 ta;
	Array<RaycastHit> hits(ta);

	world->raycast(stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_float(4)
		, mode
		, hits
		);

	stack.push_table();
	for (uint32_t i = 0; i < array::size(hits); ++i)
	{
		stack.push_key_begin(i+1);
		stack.push_actor(hits[i].actor);
		stack.push_key_end();
	}

	return 1;
}

static int physics_world_enable_debug_drawing(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->enable_debug_drawing(stack.get_bool(2));
	return 0;
}

static int physics_world_tostring(lua_State* L)
{
	LuaStack stack(L);
	PhysicsWorld* pw = stack.get_physics_world(1);
	stack.push_fstring("PhysicsWorld (%p)", pw);
	return 1;
}

static int sound_world_stop_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->stop_all();
	return 0;
}

static int sound_world_pause_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->pause_all();
	return 0;
}

static int sound_world_resume_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->resume_all();
	return 0;
}

static int sound_world_is_playing(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_sound_world(1)->is_playing(stack.get_sound_instance_id(2)));
	return 1;
}

static int sound_world_tostring(lua_State* L)
{
	LuaStack stack(L);
	SoundWorld* sw = stack.get_sound_world(1);
	stack.push_fstring("SoundWorld (%p)", sw);
	return 1;
}

static int device_platform(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->platform());
	return 1;
}

static int device_architecture(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->architecture());
	return 1;
}

static int device_version(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->version());
	return 1;
}

static int device_last_delta_time(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(device()->last_delta_time());
	return 1;
}

static int device_quit(lua_State* /*L*/)
{
	device()->quit();
	return 0;
}

static int device_resolution(lua_State* L)
{
	LuaStack stack(L);
	uint16_t w, h;
	device()->resolution(w, h);
	stack.push_int(w);
	stack.push_int(h);
	return 2;
}

static int device_create_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_world(device()->create_world());
	return 1;
}

static int device_destroy_world(lua_State* L)
{
	LuaStack stack(L);
	device()->destroy_world(*stack.get_world(1));
	return 0;
}

static int device_render_world(lua_State* L)
{
	LuaStack stack(L);
	device()->render_world(*stack.get_world(1), stack.get_camera(2));
	return 0;
}

static int device_create_resource_package(lua_State* L)
{
	LuaStack stack(L);
	stack.push_resource_package(device()->create_resource_package(stack.get_resource_id(1)));
	return 1;
}

static int device_destroy_resource_package(lua_State* L)
{
	LuaStack stack(L);
	device()->destroy_resource_package(*stack.get_resource_package(1));
	return 0;
}

static int device_console_send(lua_State* L)
{
	using namespace string_stream;
	LuaStack stack(L);

	TempAllocator1024 alloc;
	StringStream json(alloc);

	json << "{";
	/* table is in the stack at index 'i' */
	stack.push_nil();  /* first key */
	while (stack.next(1) != 0)
	{
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		json << "\"" << stack.get_string(-2) << "\":\"" << stack.get_string(-1) << "\",";
		/* removes 'value'; keeps 'key' for next iteration */
		stack.pop(1);
	}
	/* pop key */
	stack.pop(1);
	json << "}";

	console_server_globals::console()->send(c_str(json));
	return 0;
}

static int device_can_get(lua_State* L)
{
	LuaStack stack(L);
	const StringId64 type(stack.get_string(1));
	stack.push_bool(device()->resource_manager()->can_get(type, stack.get_resource_id(2)));
	return 1;
}

static int device_enable_resource_autoload(lua_State* L)
{
	LuaStack stack(L);
	device()->resource_manager()->enable_autoload(stack.get_bool(1));
	return 0;
}

static int profiler_enter_scope(lua_State* L)
{
	LuaStack stack(L);
	profiler::enter_profile_scope(stack.get_string(1));
	return 0;
}

static int profiler_leave_scope(lua_State* L)
{
	LuaStack stack(L);
	profiler::leave_profile_scope();
	return 0;
}

static int profiler_record(lua_State* L)
{
	LuaStack stack(L);

	const char* name = stack.get_string(1);

	if (stack.is_number(2))
		profiler::record_float(name, stack.get_float(2));
	else
		profiler::record_vector3(name, stack.get_vector3(2));

	return 0;
}

static int debug_line_add_line(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_line(stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_color4(4)
		);
	return 0;
}

static int debug_line_add_axes(lua_State* L)
{
	LuaStack stack(L);
	const float len = stack.num_args() == 3
		? stack.get_float(3)
		: 1.0f
		;
	stack.get_debug_line(1)->add_axes(stack.get_matrix4x4(2), len);
	return 0;
}

static int debug_line_add_circle(lua_State* L)
{
	LuaStack stack(L);
	const uint32_t segments = stack.num_args() >= 6
		? stack.get_int(6)
		: DebugLine::NUM_SEGMENTS
		;
	stack.get_debug_line(1)->add_circle(stack.get_vector3(2)
		, stack.get_float(3)
		, stack.get_vector3(4)
		, stack.get_color4(5)
		, segments
		);
	return 0;
}

static int debug_line_add_cone(lua_State* L)
{
	LuaStack stack(L);
	const uint32_t segments = stack.num_args() >= 6
		? stack.get_int(6)
		: DebugLine::NUM_SEGMENTS
		;
	stack.get_debug_line(1)->add_cone(stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_float(4)
		, stack.get_color4(5)
		, segments
		);
	return 0;
}

static int debug_line_add_sphere(lua_State* L)
{
	LuaStack stack(L);
	const uint32_t segments = stack.num_args() >= 5
		? stack.get_int(5)
		: DebugLine::NUM_SEGMENTS
		;
	stack.get_debug_line(1)->add_sphere(stack.get_vector3(2)
		, stack.get_float(3)
		, stack.get_color4(4)
		, segments
		);
	return 0;
}

static int debug_line_add_obb(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_obb(stack.get_matrix4x4(2)
		, stack.get_vector3(3)
		, stack.get_color4(4)
		);
	return 0;
}

static int debug_line_reset(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->reset();
	return 0;
}

static int debug_line_submit(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->submit();
	return 0;
}

static int debug_line_tostring(lua_State* L)
{
	LuaStack stack(L);
	stack.push_fstring("DebugLine (%p)", stack.get_debug_line(1));
	return 1;
}

static int resource_package_load(lua_State* L)
{
	LuaStack stack(L);
	stack.get_resource_package(1)->load();
	return 0;
}

static int resource_package_unload(lua_State* L)
{
	LuaStack stack(L);
	stack.get_resource_package(1)->unload();
	return 0;
}

static int resource_package_flush(lua_State* L)
{
	LuaStack stack(L);
	stack.get_resource_package(1)->flush();
	return 0;
}

static int resource_package_has_loaded(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_resource_package(1)->has_loaded());
	return 1;
}

static int resource_package_tostring(lua_State* L)
{
	LuaStack stack(L);
	ResourcePackage* package = stack.get_resource_package(1);
	stack.push_fstring("ResourcePackage (%p)", package);
	return 1;
}

static int material_set_float(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_float(stack.get_string(2), stack.get_float(3));
	return 0;
}

static int material_set_vector2(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_vector2(stack.get_string(2), stack.get_vector2(3));
	return 0;
}

static int material_set_vector3(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_vector3(stack.get_string(2), stack.get_vector3(3));
	return 0;
}

static int gui_resolution(lua_State* L)
{
	LuaStack stack(L);
	const Vector2 resolution = stack.get_gui(1)->resolution();
	stack.push_int(resolution.x);
	stack.push_int(resolution.y);
	return 2;
}

static int gui_move(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->move(stack.get_vector2(2));
	return 0;
}

static int gui_screen_to_gui(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(stack.get_gui(1)->screen_to_gui(stack.get_vector2(2)));
	return 1;
}

static int gui_draw_rectangle(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->draw_rectangle(stack.get_vector3(2), stack.get_vector2(3),
		stack.get_color4(4));
	return 0;
}

static int gui_draw_image(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->draw_image(stack.get_string(2), stack.get_vector3(3),
		stack.get_vector2(4), stack.get_color4(5));
	return 0;
}

static int gui_draw_image_uv(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->draw_image_uv(stack.get_string(2), stack.get_vector3(3),
		stack.get_vector2(4), stack.get_vector2(5), stack.get_vector2(6), stack.get_color4(7));
	return 0;
}

static int gui_draw_text(lua_State* L)
{
	LuaStack stack(L);
	stack.get_gui(1)->draw_text(stack.get_string(2), stack.get_string(3), stack.get_int(4),
		stack.get_vector3(5), stack.get_color4(6));
	return 0;
}

static int window_show(lua_State* L)
{
	LuaStack stack(L);
	// window()->show();
	return 0;
}

static int window_hide(lua_State* L)
{
	LuaStack stack(L);
	// window()->hide();
	return 0;
}

static int window_resize(lua_State* L)
{
	LuaStack stack(L);
	// window()->resize(stack.get_int(1), stack.get_int(2));
	return 0;
}

static int window_move(lua_State* L)
{
	LuaStack stack(L);
	// window()->move(stack.get_int(1), stack.get_int(2));
	return 0;
}

static int window_minimize(lua_State* /*L*/)
{
	// window()->minimize();
	return 0;
}

static int window_restore(lua_State* /*L*/)
{
	// window()->restore();
	return 0;
}

static int window_is_resizable(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(/*window()->is_resizable()*/ false);
	return 1;
}

static int window_set_resizable(lua_State* L)
{
	LuaStack stack(L);
	// window()->set_resizable(stack.get_bool(1));
	return 0;
}

static int window_title(lua_State* L)
{
	LuaStack stack(L);
	// stack.push_string(window()->title());
	stack.push_string("");
	return 1;
}

static int window_set_title(lua_State* L)
{
	LuaStack stack(L);
	// window()->set_title(stack.get_string(1));
	return 0;
}

void load_api(LuaEnvironment& env)
{
	env.load_module_function("Math", "ray_plane_intersection",  math_ray_plane_intersection);
	env.load_module_function("Math", "ray_disc_intersection",   math_ray_disc_intersection);
	env.load_module_function("Math", "ray_sphere_intersection", math_ray_sphere_intersection);
	env.load_module_function("Math", "ray_obb_intersection",    math_ray_obb_intersection);

	env.load_module_function("Vector3", "new",              vector3_new);
	env.load_module_function("Vector3", "x",                vector3_x);
	env.load_module_function("Vector3", "y",                vector3_y);
	env.load_module_function("Vector3", "z",                vector3_z);
	env.load_module_function("Vector3", "set_x",            vector3_set_x);
	env.load_module_function("Vector3", "set_y",            vector3_set_y);
	env.load_module_function("Vector3", "set_z",            vector3_set_z);
	env.load_module_function("Vector3", "elements",         vector3_elements);
	env.load_module_function("Vector3", "add",              vector3_add);
	env.load_module_function("Vector3", "subtract",         vector3_subtract);
	env.load_module_function("Vector3", "multiply",         vector3_multiply);
	env.load_module_function("Vector3", "dot",              vector3_dot);
	env.load_module_function("Vector3", "cross",            vector3_cross);
	env.load_module_function("Vector3", "equal",            vector3_equal);
	env.load_module_function("Vector3", "length",           vector3_length);
	env.load_module_function("Vector3", "length_squared",   vector3_length_squared);
	env.load_module_function("Vector3", "set_length",       vector3_set_length);
	env.load_module_function("Vector3", "normalize",        vector3_normalize);
	env.load_module_function("Vector3", "distance",         vector3_distance);
	env.load_module_function("Vector3", "distance_squared", vector3_distance_squared);
	env.load_module_function("Vector3", "angle",            vector3_angle);
	env.load_module_function("Vector3", "max",              vector3_max);
	env.load_module_function("Vector3", "min",              vector3_min);
	env.load_module_function("Vector3", "lerp",             vector3_lerp);
	env.load_module_function("Vector3", "forward",          vector3_forward);
	env.load_module_function("Vector3", "backward",         vector3_backward);
	env.load_module_function("Vector3", "left",             vector3_left);
	env.load_module_function("Vector3", "right",            vector3_right);
	env.load_module_function("Vector3", "up",               vector3_up);
	env.load_module_function("Vector3", "down",             vector3_down);
	env.load_module_function("Vector3", "zero",             vector3_zero);
	env.load_module_function("Vector3", "to_string",        vector3_to_string);

	env.load_module_constructor("Vector3", vector3_ctor);

	env.load_module_function("Vector2", "new",            vector2_new);
	env.load_module_constructor("Vector2", vector2_ctor);

	env.load_module_function("Vector3Box", "new",        vector3box_new);
	env.load_module_function("Vector3Box", "store",      vector3box_store);
	env.load_module_function("Vector3Box", "unbox",      vector3box_unbox);
	env.load_module_function("Vector3Box", "__index",    "Vector3Box");
	env.load_module_function("Vector3Box", "__tostring", vector3box_tostring);

	env.load_module_constructor("Vector3Box", vector3box_ctor);

	env.load_module_function("Matrix4x4", "new",                         matrix4x4_new);
	env.load_module_function("Matrix4x4", "from_quaternion",             matrix4x4_from_quaternion);
	env.load_module_function("Matrix4x4", "from_translation",            matrix4x4_from_translation);
	env.load_module_function("Matrix4x4", "from_quaternion_translation", matrix4x4_from_quaternion_translation);
	env.load_module_function("Matrix4x4", "from_axes",                   matrix4x4_from_axes);
	env.load_module_function("Matrix4x4", "copy",                        matrix4x4_copy);
	env.load_module_function("Matrix4x4", "add",                         matrix4x4_add);
	env.load_module_function("Matrix4x4", "subtract",                    matrix4x4_subtract);
	env.load_module_function("Matrix4x4", "multiply",                    matrix4x4_multiply);
	env.load_module_function("Matrix4x4", "transpose",                   matrix4x4_transpose);
	env.load_module_function("Matrix4x4", "determinant",                 matrix4x4_determinant);
	env.load_module_function("Matrix4x4", "invert",                      matrix4x4_invert);
	env.load_module_function("Matrix4x4", "x",                           matrix4x4_x);
	env.load_module_function("Matrix4x4", "y",                           matrix4x4_y);
	env.load_module_function("Matrix4x4", "z",                           matrix4x4_z);
	env.load_module_function("Matrix4x4", "set_x",                       matrix4x4_set_x);
	env.load_module_function("Matrix4x4", "set_y",                       matrix4x4_set_y);
	env.load_module_function("Matrix4x4", "set_z",                       matrix4x4_set_z);
	env.load_module_function("Matrix4x4", "translation",                 matrix4x4_translation);
	env.load_module_function("Matrix4x4", "set_translation",             matrix4x4_set_translation);
	env.load_module_function("Matrix4x4", "rotation",                    matrix4x4_rotation);
	env.load_module_function("Matrix4x4", "set_rotation",                matrix4x4_set_rotation);
	env.load_module_function("Matrix4x4", "identity",                    matrix4x4_identity);
	env.load_module_function("Matrix4x4", "transform",                   matrix4x4_transform);
	env.load_module_function("Matrix4x4", "to_string",                   matrix4x4_to_string);

	env.load_module_constructor("Matrix4x4", matrix4x4_ctor);

	env.load_module_function("Matrix4x4Box", "new",        matrix4x4box_new);
	env.load_module_function("Matrix4x4Box", "store",      matrix4x4box_store);
	env.load_module_function("Matrix4x4Box", "unbox",      matrix4x4box_unbox);
	env.load_module_function("Matrix4x4Box", "__index",    "Matrix4x4Box");
	env.load_module_function("Matrix4x4Box", "__tostring", matrix4x4box_tostring);

	env.load_module_constructor("Matrix4x4Box", matrix4x4box_ctor);

	env.load_module_function("Quaternion", "new",                quaternion_new);
	env.load_module_function("Quaternion", "negate",             quaternion_negate);
	env.load_module_function("Quaternion", "identity",           quaternion_identity);
	env.load_module_function("Quaternion", "multiply",           quaternion_multiply);
	env.load_module_function("Quaternion", "multiply_by_scalar", quaternion_multiply_by_scalar);
	env.load_module_function("Quaternion", "dot",                quaternion_dot);
	env.load_module_function("Quaternion", "length",             quaternion_length);
	env.load_module_function("Quaternion", "normalize",          quaternion_normalize);
	env.load_module_function("Quaternion", "conjugate",          quaternion_conjugate);
	env.load_module_function("Quaternion", "inverse",            quaternion_inverse);
	env.load_module_function("Quaternion", "power",              quaternion_power);
	env.load_module_function("Quaternion", "elements",           quaternion_elements);
	env.load_module_function("Quaternion", "look",               quaternion_look);
	env.load_module_function("Quaternion", "right",              quaternion_right);
	env.load_module_function("Quaternion", "up",                 quaternion_up);
	env.load_module_function("Quaternion", "forward",            quaternion_forward);
	env.load_module_function("Quaternion", "lerp",               quaternion_lerp);
	env.load_module_function("Quaternion", "to_string",          quaternion_to_string);

	env.load_module_constructor("Quaternion", quaternion_ctor);

	env.load_module_function("QuaternionBox", "new",        quaternionbox_new);
	env.load_module_function("QuaternionBox", "store",      quaternionbox_store);
	env.load_module_function("QuaternionBox", "unbox",      quaternionbox_unbox);
	env.load_module_function("QuaternionBox", "__index",    "QuaternionBox");
	env.load_module_function("QuaternionBox", "__tostring", quaternionbox_tostring);

	env.load_module_constructor("QuaternionBox", quaternionbox_ctor);

	env.load_module_function("Color4", "new",       color4_new);
	env.load_module_function("Color4", "to_string", quaternion_to_string);

	env.load_module_constructor("Color4",     color4_ctor);

	env.load_module_function("Lightuserdata_mt", "__add",      lightuserdata_add);
	env.load_module_function("Lightuserdata_mt", "__sub",      lightuserdata_sub);
	env.load_module_function("Lightuserdata_mt", "__mul",      lightuserdata_mul);
	env.load_module_function("Lightuserdata_mt", "__unm",      lightuserdata_unm);
	env.load_module_function("Lightuserdata_mt", "__index",    lightuserdata_index);
	env.load_module_function("Lightuserdata_mt", "__newindex", lightuserdata_newindex);

	env.load_module_function("Keyboard", "name",         KEYBOARD_FN(name));
	env.load_module_function("Keyboard", "connected",    KEYBOARD_FN(connected));
	env.load_module_function("Keyboard", "num_buttons",  KEYBOARD_FN(num_buttons));
	env.load_module_function("Keyboard", "num_axes",     KEYBOARD_FN(num_axes));
	env.load_module_function("Keyboard", "pressed",      KEYBOARD_FN(pressed));
	env.load_module_function("Keyboard", "released",     KEYBOARD_FN(released));
	env.load_module_function("Keyboard", "any_pressed",  KEYBOARD_FN(any_pressed));
	env.load_module_function("Keyboard", "any_released", KEYBOARD_FN(any_released));
	env.load_module_function("Keyboard", "button_name",  KEYBOARD_FN(button_name));
	env.load_module_function("Keyboard", "button_id",    KEYBOARD_FN(button_id));

	env.load_module_function("Mouse", "name",         MOUSE_FN(name));
	env.load_module_function("Mouse", "connected",    MOUSE_FN(connected));
	env.load_module_function("Mouse", "num_buttons",  MOUSE_FN(num_buttons));
	env.load_module_function("Mouse", "num_axes",     MOUSE_FN(num_axes));
	env.load_module_function("Mouse", "pressed",      MOUSE_FN(pressed));
	env.load_module_function("Mouse", "released",     MOUSE_FN(released));
	env.load_module_function("Mouse", "any_pressed",  MOUSE_FN(any_pressed));
	env.load_module_function("Mouse", "any_released", MOUSE_FN(any_released));
	env.load_module_function("Mouse", "axis",         MOUSE_FN(axis));
	env.load_module_function("Mouse", "button_name",  MOUSE_FN(button_name));
	env.load_module_function("Mouse", "axis_name",    MOUSE_FN(axis_name));
	env.load_module_function("Mouse", "button_id",    MOUSE_FN(button_id));
	env.load_module_function("Mouse", "axis_id",      MOUSE_FN(axis_id));

	env.load_module_function("Touch", "name",         TOUCH_FN(name));
	env.load_module_function("Touch", "connected",    TOUCH_FN(connected));
	env.load_module_function("Touch", "num_buttons",  TOUCH_FN(num_buttons));
	env.load_module_function("Touch", "num_axes",     TOUCH_FN(num_axes));
	env.load_module_function("Touch", "pressed",      TOUCH_FN(pressed));
	env.load_module_function("Touch", "released",     TOUCH_FN(released));
	env.load_module_function("Touch", "any_pressed",  TOUCH_FN(any_pressed));
	env.load_module_function("Touch", "any_released", TOUCH_FN(any_released));
	env.load_module_function("Touch", "axis",         TOUCH_FN(axis));
	env.load_module_function("Touch", "button_name",  TOUCH_FN(button_name));
	env.load_module_function("Touch", "axis_name",    TOUCH_FN(axis_name));
	env.load_module_function("Touch", "button_id",    TOUCH_FN(button_id));
	env.load_module_function("Touch", "axis_id",      TOUCH_FN(axis_id));

	env.load_module_function("Pad1", "name",         JOYPAD_FN(0, name));
	env.load_module_function("Pad1", "connected",    JOYPAD_FN(0, connected));
	env.load_module_function("Pad1", "num_buttons",  JOYPAD_FN(0, num_buttons));
	env.load_module_function("Pad1", "num_axes",     JOYPAD_FN(0, num_axes));
	env.load_module_function("Pad1", "pressed",      JOYPAD_FN(0, pressed));
	env.load_module_function("Pad1", "released",     JOYPAD_FN(0, released));
	env.load_module_function("Pad1", "any_pressed",  JOYPAD_FN(0, any_pressed));
	env.load_module_function("Pad1", "any_released", JOYPAD_FN(0, any_released));
	env.load_module_function("Pad1", "axis",         JOYPAD_FN(0, axis));
	env.load_module_function("Pad1", "button_name",  JOYPAD_FN(0, button_name));
	env.load_module_function("Pad1", "axis_name",    JOYPAD_FN(0, axis_name));
	env.load_module_function("Pad1", "button_id",    JOYPAD_FN(0, button_id));
	env.load_module_function("Pad1", "axis_id",      JOYPAD_FN(0, axis_id));

	env.load_module_function("Pad2", "name",         JOYPAD_FN(1, name));
	env.load_module_function("Pad2", "connected",    JOYPAD_FN(1, connected));
	env.load_module_function("Pad2", "num_buttons",  JOYPAD_FN(1, num_buttons));
	env.load_module_function("Pad2", "num_axes",     JOYPAD_FN(1, num_axes));
	env.load_module_function("Pad2", "pressed",      JOYPAD_FN(1, pressed));
	env.load_module_function("Pad2", "released",     JOYPAD_FN(1, released));
	env.load_module_function("Pad2", "any_pressed",  JOYPAD_FN(1, any_pressed));
	env.load_module_function("Pad2", "any_released", JOYPAD_FN(1, any_released));
	env.load_module_function("Pad2", "axis",         JOYPAD_FN(1, axis));
	env.load_module_function("Pad2", "button_name",  JOYPAD_FN(1, button_name));
	env.load_module_function("Pad2", "axis_name",    JOYPAD_FN(1, axis_name));
	env.load_module_function("Pad2", "button_id",    JOYPAD_FN(1, button_id));
	env.load_module_function("Pad2", "axis_id",      JOYPAD_FN(1, axis_id));

	env.load_module_function("Pad3", "name",         JOYPAD_FN(2, name));
	env.load_module_function("Pad3", "connected",    JOYPAD_FN(2, connected));
	env.load_module_function("Pad3", "num_buttons",  JOYPAD_FN(2, num_buttons));
	env.load_module_function("Pad3", "num_axes",     JOYPAD_FN(2, num_axes));
	env.load_module_function("Pad3", "pressed",      JOYPAD_FN(2, pressed));
	env.load_module_function("Pad3", "released",     JOYPAD_FN(2, released));
	env.load_module_function("Pad3", "any_pressed",  JOYPAD_FN(2, any_pressed));
	env.load_module_function("Pad3", "any_released", JOYPAD_FN(2, any_released));
	env.load_module_function("Pad3", "axis",         JOYPAD_FN(2, axis));
	env.load_module_function("Pad3", "button_name",  JOYPAD_FN(2, button_name));
	env.load_module_function("Pad3", "axis_name",    JOYPAD_FN(2, axis_name));
	env.load_module_function("Pad3", "button_id",    JOYPAD_FN(2, button_id));
	env.load_module_function("Pad3", "axis_id",      JOYPAD_FN(2, axis_id));

	env.load_module_function("Pad4", "name",         JOYPAD_FN(3, name));
	env.load_module_function("Pad4", "connected",    JOYPAD_FN(3, connected));
	env.load_module_function("Pad4", "num_buttons",  JOYPAD_FN(3, num_buttons));
	env.load_module_function("Pad4", "num_axes",     JOYPAD_FN(3, num_axes));
	env.load_module_function("Pad4", "pressed",      JOYPAD_FN(3, pressed));
	env.load_module_function("Pad4", "released",     JOYPAD_FN(3, released));
	env.load_module_function("Pad4", "any_pressed",  JOYPAD_FN(3, any_pressed));
	env.load_module_function("Pad4", "any_released", JOYPAD_FN(3, any_released));
	env.load_module_function("Pad4", "axis",         JOYPAD_FN(3, axis));
	env.load_module_function("Pad4", "button_name",  JOYPAD_FN(3, button_name));
	env.load_module_function("Pad4", "axis_name",    JOYPAD_FN(3, axis_name));
	env.load_module_function("Pad4", "button_id",    JOYPAD_FN(3, button_id));
	env.load_module_function("Pad4", "axis_id",      JOYPAD_FN(3, axis_id));

	env.load_module_function("World", "spawn_unit",                      world_spawn_unit);
	env.load_module_function("World", "destroy_unit",                    world_destroy_unit);
	env.load_module_function("World", "num_units",                       world_num_units);
	env.load_module_function("World", "units",                           world_units);

	env.load_module_function("World", "camera",                          world_camera);
	env.load_module_function("World", "set_camera_projection_type",      camera_set_projection_type);
	env.load_module_function("World", "camera_projection_type",          camera_projection_type);
	env.load_module_function("World", "camera_fov",                      camera_fov);
	env.load_module_function("World", "set_camera_fov",                  camera_set_fov);
	env.load_module_function("World", "camera_aspect",                   camera_aspect);
	env.load_module_function("World", "set_camera_aspect",               camera_set_aspect);
	env.load_module_function("World", "camera_near_clip_distance",       camera_near_clip_distance);
	env.load_module_function("World", "set_camera_near_clip_distance",   camera_set_near_clip_distance);
	env.load_module_function("World", "camera_far_clip_distance",        camera_far_clip_distance);
	env.load_module_function("World", "set_camera_far_clip_distance",    camera_set_far_clip_distance);
	env.load_module_function("World", "set_camera_orthographic_metrics", camera_set_orthographic_metrics);
	env.load_module_function("World", "set_camera_viewport_metrics",     camera_set_viewport_metrics);
	env.load_module_function("World", "camera_screen_to_world",          camera_screen_to_world);
	env.load_module_function("World", "camera_world_to_screen",          camera_world_to_screen);

	env.load_module_function("World", "update_animations",               world_update_animations);
	env.load_module_function("World", "update_scene",                    world_update_scene);
	env.load_module_function("World", "update",                          world_update);
	env.load_module_function("World", "play_sound",                      world_play_sound);
	env.load_module_function("World", "stop_sound",                      world_stop_sound);
	env.load_module_function("World", "link_sound",                      world_link_sound);
	env.load_module_function("World", "set_listener_pose",               world_set_listener_pose);
	env.load_module_function("World", "set_sound_position",              world_set_sound_position);
	env.load_module_function("World", "set_sound_range",                 world_set_sound_range);
	env.load_module_function("World", "set_sound_volume",                world_set_sound_volume);
	env.load_module_function("World", "create_debug_line",               world_create_debug_line);
	env.load_module_function("World", "destroy_debug_line",              world_destroy_debug_line);
	env.load_module_function("World", "load_level",                      world_load_level);
	env.load_module_function("World", "scene_graph",                     world_scene_graph);
	env.load_module_function("World", "render_world",                    world_render_world);
	env.load_module_function("World", "physics_world",                   world_physics_world);
	env.load_module_function("World", "sound_world",                     world_sound_world);
	env.load_module_function("World", "__index",                         "World");
	env.load_module_function("World", "__tostring",                      world_tostring);

	env.load_module_function("SceneGraph", "create",              scene_graph_create);
	env.load_module_function("SceneGraph", "destroy",             scene_graph_destroy);
	env.load_module_function("SceneGraph", "transform_instances", scene_graph_transform_instances);
	env.load_module_function("SceneGraph", "local_position",      scene_graph_local_position);
	env.load_module_function("SceneGraph", "local_rotation",      scene_graph_local_rotation);
	env.load_module_function("SceneGraph", "local_scale",         scene_graph_local_scale);
	env.load_module_function("SceneGraph", "local_pose",          scene_graph_local_pose);
	env.load_module_function("SceneGraph", "world_position",      scene_graph_world_position);
	env.load_module_function("SceneGraph", "world_rotation",      scene_graph_world_rotation);
	env.load_module_function("SceneGraph", "world_pose",          scene_graph_world_pose);
	env.load_module_function("SceneGraph", "set_local_position",  scene_graph_set_local_position);
	env.load_module_function("SceneGraph", "set_local_rotation",  scene_graph_set_local_rotation);
	env.load_module_function("SceneGraph", "set_local_scale",     scene_graph_set_local_scale);
	env.load_module_function("SceneGraph", "set_local_pose",      scene_graph_set_local_pose);
	env.load_module_function("SceneGraph", "link",                scene_graph_link);
	env.load_module_function("SceneGraph", "unlink",              scene_graph_unlink);

	env.load_module_function("UnitManager", "create", unit_manager_create);
	env.load_module_function("UnitManager", "alive",  unit_manager_alive);

	env.load_module_function("RenderWorld", "create_mesh",          render_world_create_mesh);
	env.load_module_function("RenderWorld", "destroy_mesh",         render_world_destroy_mesh);
	env.load_module_function("RenderWorld", "mesh_instances",       render_world_mesh_instances);
	env.load_module_function("RenderWorld", "mesh_obb",             render_world_mesh_obb);
	env.load_module_function("RenderWorld", "set_mesh_visible",     render_world_set_mesh_visible);
	env.load_module_function("RenderWorld", "create_sprite",        render_world_create_sprite);
	env.load_module_function("RenderWorld", "destroy_sprite",       render_world_destroy_sprite);
	env.load_module_function("RenderWorld", "sprite_instances",     render_world_sprite_instances);
	env.load_module_function("RenderWorld", "set_sprite_frame",     render_world_set_sprite_frame);
	env.load_module_function("RenderWorld", "set_sprite_visible",   render_world_set_sprite_visible);
	env.load_module_function("RenderWorld", "create_light",         render_world_create_light);
	env.load_module_function("RenderWorld", "destroy_light",        render_world_destroy_light);
	env.load_module_function("RenderWorld", "light_instances",      render_world_light_instances);
	env.load_module_function("RenderWorld", "light_type",           render_world_light_type);
	env.load_module_function("RenderWorld", "light_color",          render_world_light_color);
	env.load_module_function("RenderWorld", "light_range",          render_world_light_range);
	env.load_module_function("RenderWorld", "light_intensity",      render_world_light_intensity);
	env.load_module_function("RenderWorld", "light_spot_angle",     render_world_light_spot_angle);
	env.load_module_function("RenderWorld", "set_light_type",       render_world_set_light_type);
	env.load_module_function("RenderWorld", "set_light_color",      render_world_set_light_color);
	env.load_module_function("RenderWorld", "set_light_range",      render_world_set_light_range);
	env.load_module_function("RenderWorld", "set_light_intensity",  render_world_set_light_intensity);
	env.load_module_function("RenderWorld", "set_light_spot_angle", render_world_set_light_spot_angle);
	env.load_module_function("RenderWorld", "enable_debug_drawing", render_world_enable_debug_drawing);

	env.load_module_function("PhysicsWorld", "actor_instances",               physics_world_actor_instances);
	env.load_module_function("PhysicsWorld", "actor_world_position",          physics_world_actor_world_position);
	env.load_module_function("PhysicsWorld", "actor_world_rotation",          physics_world_actor_world_rotation);
	env.load_module_function("PhysicsWorld", "actor_world_pose",              physics_world_actor_world_pose);
	env.load_module_function("PhysicsWorld", "teleport_actor_world_position", physics_world_teleport_actor_world_position);
	env.load_module_function("PhysicsWorld", "teleport_actor_world_rotation", physics_world_teleport_actor_world_rotation);
	env.load_module_function("PhysicsWorld", "teleport_actor_world_pose",     physics_world_teleport_actor_world_pose);
	env.load_module_function("PhysicsWorld", "actor_center_of_mass",          physics_world_actor_center_of_mass);
	env.load_module_function("PhysicsWorld", "enable_actor_gravity",          physics_world_enable_actor_gravity);
	env.load_module_function("PhysicsWorld", "disable_actor_gravity",         physics_world_disable_actor_gravity);
	env.load_module_function("PhysicsWorld", "enable_actor_collision",        physics_world_enable_actor_collision);
	env.load_module_function("PhysicsWorld", "disable_actor_collision",       physics_world_disable_actor_collision);
	env.load_module_function("PhysicsWorld", "set_actor_collision_filter",    physics_world_set_actor_collision_filter);
	env.load_module_function("PhysicsWorld", "set_actor_kinematic",           physics_world_set_actor_kinematic);
	env.load_module_function("PhysicsWorld", "move_actor",                    physics_world_move_actor);
	env.load_module_function("PhysicsWorld", "is_static",                     physics_world_is_static);
	env.load_module_function("PhysicsWorld", "is_dynamic",                    physics_world_is_dynamic);
	env.load_module_function("PhysicsWorld", "is_kinematic",                  physics_world_is_kinematic);
	env.load_module_function("PhysicsWorld", "is_nonkinematic",               physics_world_is_nonkinematic);
	env.load_module_function("PhysicsWorld", "actor_linear_damping",          physics_world_actor_linear_damping);
	env.load_module_function("PhysicsWorld", "set_actor_linear_damping",      physics_world_set_actor_linear_damping);
	env.load_module_function("PhysicsWorld", "actor_angular_damping",         physics_world_actor_angular_damping);
	env.load_module_function("PhysicsWorld", "set_actor_angular_damping",     physics_world_set_actor_angular_damping);
	env.load_module_function("PhysicsWorld", "actor_linear_velocity",         physics_world_actor_linear_velocity);
	env.load_module_function("PhysicsWorld", "set_actor_linear_velocity",     physics_world_set_actor_linear_velocity);
	env.load_module_function("PhysicsWorld", "actor_angular_velocity",        physics_world_actor_angular_velocity);
	env.load_module_function("PhysicsWorld", "set_actor_angular_velocity",    physics_world_set_actor_angular_velocity);
	env.load_module_function("PhysicsWorld", "add_actor_impulse",             physics_world_add_actor_impulse);
	env.load_module_function("PhysicsWorld", "add_actor_impulse_at",          physics_world_add_actor_impulse_at);
	env.load_module_function("PhysicsWorld", "add_actor_torque_impulse",      physics_world_add_actor_torque_impulse);
	env.load_module_function("PhysicsWorld", "push_actor",                    physics_world_push_actor);
	env.load_module_function("PhysicsWorld", "push_actor_at",                 physics_world_push_actor_at);
	env.load_module_function("PhysicsWorld", "is_sleeping",                   physics_world_is_sleeping);
	env.load_module_function("PhysicsWorld", "wake_up",                       physics_world_wake_up);
	env.load_module_function("PhysicsWorld", "controller_instances",          physics_world_controller_instances);
	env.load_module_function("PhysicsWorld", "move_controller",               physics_world_move_controller);
	env.load_module_function("PhysicsWorld", "create_joint",                  physics_world_create_joint);
	env.load_module_function("PhysicsWorld", "gravity",                       physics_world_gravity);
	env.load_module_function("PhysicsWorld", "set_gravity",                   physics_world_set_gravity);
	env.load_module_function("PhysicsWorld", "raycast",                       physics_world_raycast);
	env.load_module_function("PhysicsWorld", "enable_debug_drawing",          physics_world_enable_debug_drawing);
	env.load_module_function("PhysicsWorld", "__index",                       "PhysicsWorld");
	env.load_module_function("PhysicsWorld", "__tostring",                    physics_world_tostring);

	env.load_module_function("SoundWorld", "stop_all",   sound_world_stop_all);
	env.load_module_function("SoundWorld", "pause_all",  sound_world_pause_all);
	env.load_module_function("SoundWorld", "resume_all", sound_world_resume_all);
	env.load_module_function("SoundWorld", "is_playing", sound_world_is_playing);
	env.load_module_function("SoundWorld", "__index",    "SoundWorld");
	env.load_module_function("SoundWorld", "__tostring", sound_world_tostring);

	env.load_module_function("Device", "platform",                 device_platform);
	env.load_module_function("Device", "architecture",             device_architecture);
	env.load_module_function("Device", "version",                  device_version);
	env.load_module_function("Device", "last_delta_time",          device_last_delta_time);
	env.load_module_function("Device", "quit",                     device_quit);
	env.load_module_function("Device", "resolution",               device_resolution);
	env.load_module_function("Device", "create_world",             device_create_world);
	env.load_module_function("Device", "destroy_world",            device_destroy_world);
	env.load_module_function("Device", "render_world",             device_render_world);
	env.load_module_function("Device", "create_resource_package",  device_create_resource_package);
	env.load_module_function("Device", "destroy_resource_package", device_destroy_resource_package);
	env.load_module_function("Device", "console_send",             device_console_send);
	env.load_module_function("Device", "can_get",                  device_can_get);
	env.load_module_function("Device", "enable_resource_autoload", device_enable_resource_autoload);

	env.load_module_function("Profiler", "enter_scope", profiler_enter_scope);
	env.load_module_function("Profiler", "leave_scope", profiler_leave_scope);
	env.load_module_function("Profiler", "record",      profiler_record);

	env.load_module_function("DebugLine", "add_line",   debug_line_add_line);
	env.load_module_function("DebugLine", "add_axes",   debug_line_add_axes);
	env.load_module_function("DebugLine", "add_circle", debug_line_add_circle);
	env.load_module_function("DebugLine", "add_cone",   debug_line_add_cone);
	env.load_module_function("DebugLine", "add_sphere", debug_line_add_sphere);
	env.load_module_function("DebugLine", "add_obb",    debug_line_add_obb);
	env.load_module_function("DebugLine", "reset",      debug_line_reset);
	env.load_module_function("DebugLine", "submit",     debug_line_submit);
	env.load_module_function("DebugLine", "__index",    "DebugLine");
	env.load_module_function("DebugLine", "__tostring", debug_line_tostring);

	env.load_module_function("ResourcePackage", "load",       resource_package_load);
	env.load_module_function("ResourcePackage", "unload",     resource_package_unload);
	env.load_module_function("ResourcePackage", "flush",      resource_package_flush);
	env.load_module_function("ResourcePackage", "has_loaded", resource_package_has_loaded);
	env.load_module_function("ResourcePackage", "__index",    "ResourcePackage");
	env.load_module_function("ResourcePackage", "__tostring", resource_package_tostring);

	env.load_module_function("Material", "set_float",   material_set_float);
	env.load_module_function("Material", "set_vector2", material_set_vector2);
	env.load_module_function("Material", "set_vector3", material_set_vector3);

	env.load_module_function("Gui", "resolution",     gui_resolution);
	env.load_module_function("Gui", "move",           gui_move);
	env.load_module_function("Gui", "screen_to_gui",  gui_screen_to_gui);
	env.load_module_function("Gui", "draw_rectangle", gui_draw_rectangle);
	env.load_module_function("Gui", "draw_image",     gui_draw_image);
	env.load_module_function("Gui", "draw_image_uv",  gui_draw_image_uv);
	env.load_module_function("Gui", "draw_text",      gui_draw_text);

	env.load_module_function("Window", "show",          window_show);
	env.load_module_function("Window", "hide",          window_hide);
	env.load_module_function("Window", "resize",        window_resize);
	env.load_module_function("Window", "move",          window_move);
	env.load_module_function("Window", "minimize",      window_minimize);
	env.load_module_function("Window", "restore",       window_restore);
	env.load_module_function("Window", "is_resizable",  window_is_resizable);
	env.load_module_function("Window", "set_resizable", window_set_resizable);
	env.load_module_function("Window", "title",         window_title);
	env.load_module_function("Window", "set_title",     window_set_title);
}

} // namespace crown
