/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "aabb.h"
#include "compile_options.h"
#include "dynamic_string.h"
#include "filesystem.h"
#include "macros.h"
#include "map.h"
#include "physics_resource.h"
#include "quaternion.h"
#include "sjson.h"
#include "sphere.h"
#include "string_utils.h"
#include "world_types.h"

namespace crown
{
namespace physics_resource
{
	struct ColliderInfo
	{
		const char* name;
		ColliderType::Enum type;
	};

	static const ColliderInfo s_collider[] =
	{
		{ "sphere",      ColliderType::SPHERE      },
		{ "capsule",     ColliderType::CAPSULE     },
		{ "box",         ColliderType::BOX         },
		{ "convex_hull", ColliderType::CONVEX_HULL },
		{ "mesh",        ColliderType::MESH        },
		{ "heightfield", ColliderType::HEIGHTFIELD }
	};
	CE_STATIC_ASSERT(CE_COUNTOF(s_collider) == ColliderType::COUNT);

	struct JointInfo
	{
		const char* name;
		JointType::Enum type;
	};

	static const JointInfo s_joint[] =
	{
		{ "fixed",  JointType::FIXED  },
		{ "hinge",  JointType::HINGE  },
		{ "spring", JointType::SPRING }
	};
	CE_STATIC_ASSERT(CE_COUNTOF(s_joint) == JointType::COUNT);

	static ColliderType::Enum shape_type_to_enum(const char* type)
	{
		for (u32 i = 0; i < CE_COUNTOF(s_collider); ++i)
		{
			if (strcmp(type, s_collider[i].name) == 0)
				return s_collider[i].type;
		}

		return ColliderType::COUNT;
	}

	static JointType::Enum joint_type_to_enum(const char* type)
	{
		for (u32 i = 0; i < CE_COUNTOF(s_joint); ++i)
		{
			if (strcmp(type, s_joint[i].name) == 0)
				return s_joint[i].type;
		}

		return JointType::COUNT;
	}

	Buffer compile_controller(const char* json, CompileOptions& opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		ControllerDesc cd;
		cd.height           = sjson::parse_float    (obj["height"]);
		cd.radius           = sjson::parse_float    (obj["radius"]);
		cd.slope_limit      = sjson::parse_float    (obj["slope_limit"]);
		cd.step_offset      = sjson::parse_float    (obj["step_offset"]);
		cd.contact_offset   = sjson::parse_float    (obj["contact_offset"]);
		cd.collision_filter = sjson::parse_string_id(obj["collision_filter"]);

		Buffer buf(default_allocator());
		array::push(buf, (char*)&cd, sizeof(cd));
		return buf;
	}

	void compile_sphere(const Array<Vector3>& points, ColliderDesc& sd)
	{
		Sphere sphere;
		sphere::reset(sphere);
		sphere::add_points(sphere, array::size(points), array::begin(points));

		sd.sphere.radius = sphere.r;
	}

	void compile_capsule(const Array<Vector3>& points, ColliderDesc& sd)
	{
		AABB aabb;
		aabb::reset(aabb);
		aabb::add_points(aabb, array::size(points), array::begin(points));

		sd.capsule.radius = aabb::radius(aabb) / 2.0f;
		sd.capsule.height = (aabb.max.y - aabb.min.y) / 2.0f;
	}

	void compile_box(const Array<Vector3>& points, ColliderDesc& sd)
	{
		AABB aabb;
		aabb::reset(aabb);
		aabb::add_points(aabb, array::size(points), array::begin(points));

		sd.box.half_size = (aabb.max - aabb.min) * 0.5f;
	}

	Buffer compile_collider(const char* json, CompileOptions& opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		DynamicString type(ta);
		sjson::parse_string(obj["shape"], type);

		ColliderType::Enum st = shape_type_to_enum(type.c_str());
		RESOURCE_COMPILER_ASSERT(st != ColliderType::COUNT
			, opts
			, "Unknown shape type: '%s'"
			, type.c_str()
			);

		ColliderDesc cd;
		cd.type        = st;
		cd.shape_class = sjson::parse_string_id(obj["class"]);
		cd.material    = sjson::parse_string_id(obj["material"]);
		cd.local_tm    = MATRIX4X4_IDENTITY;
		cd.size        = 0;

		// Parse .mesh
		DynamicString scene(ta);
		DynamicString name(ta);
		sjson::parse_string(obj["scene"], scene);
		sjson::parse_string(obj["name"], name);
		RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_MESH, scene.c_str(), opts);
		scene += "." RESOURCE_EXTENSION_MESH;

		Buffer file = opts.read(scene.c_str());
		JsonObject json_mesh(ta);
		JsonObject geometries(ta);
		JsonObject geometry(ta);
		JsonObject nodes(ta);
		JsonObject node(ta);
		sjson::parse(file, json_mesh);
		sjson::parse(json_mesh["geometries"], geometries);
		sjson::parse(geometries[name.c_str()], geometry);
		sjson::parse(json_mesh["nodes"], nodes);
		sjson::parse(nodes[name.c_str()], node);

		Matrix4x4 matrix_local = sjson::parse_matrix4x4(node["matrix_local"]);
		cd.local_tm = matrix_local;

		JsonArray positions(ta);
		sjson::parse_array(geometry["position"], positions);

		Array<Vector3> points(default_allocator());
		for (u32 i = 0; i < array::size(positions); i += 3)
		{
			Vector3 p;
			p.x = sjson::parse_float(positions[i + 0]);
			p.y = sjson::parse_float(positions[i + 1]);
			p.z = sjson::parse_float(positions[i + 2]);
			array::push_back(points, p*matrix_local);
		}

		switch (cd.type)
		{
			case ColliderType::SPHERE:      compile_sphere(points, cd); break;
			case ColliderType::CAPSULE:     compile_capsule(points, cd); break;
			case ColliderType::BOX:         compile_box(points, cd); break;
			case ColliderType::CONVEX_HULL: break;
			case ColliderType::MESH:
			case ColliderType::HEIGHTFIELD:
			{
				RESOURCE_COMPILER_ASSERT(false, opts, "Not implemented yet");
				break;
			}
		}

		cd.size = cd.type == ColliderType::CONVEX_HULL ? sizeof(Vector3)*array::size(points) : 0;

		Buffer buf(default_allocator());
		array::push(buf, (char*)&cd, sizeof(cd));

		if (cd.type == ColliderType::CONVEX_HULL)
			array::push(buf, (char*)array::begin(points), sizeof(Vector3)*array::size(points));

		return buf;
	}

	Buffer compile_actor(const char* json, CompileOptions& opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		ActorResource ar;
		ar.actor_class      = sjson::parse_string_id(obj["class"]);
		ar.mass             = sjson::parse_float    (obj["mass"]);
		ar.collision_filter = sjson::parse_string_id(obj["collision_filter"]);

		ar.flags = 0;
		ar.flags |= map::has(obj, FixedString("lock_translation_x")) ? sjson::parse_bool(obj["lock_translation_x"]) : 0;
		ar.flags |= map::has(obj, FixedString("lock_translation_y")) ? sjson::parse_bool(obj["lock_translation_y"]) : 0;
		ar.flags |= map::has(obj, FixedString("lock_translation_z")) ? sjson::parse_bool(obj["lock_translation_z"]) : 0;
		ar.flags |= map::has(obj, FixedString("lock_rotation_x")) ? sjson::parse_bool(obj["lock_rotation_x"]) : 0;
		ar.flags |= map::has(obj, FixedString("lock_rotation_y")) ? sjson::parse_bool(obj["lock_rotation_y"]) : 0;
		ar.flags |= map::has(obj, FixedString("lock_rotation_z")) ? sjson::parse_bool(obj["lock_rotation_z"]) : 0;

		Buffer buf(default_allocator());
		array::push(buf, (char*)&ar, sizeof(ar));
		return buf;
	}

	Buffer compile_joint(const char* json, CompileOptions& opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		DynamicString type(ta);
		sjson::parse_string(obj["type"], type);

		JointType::Enum jt = joint_type_to_enum(type.c_str());
		RESOURCE_COMPILER_ASSERT(jt != JointType::COUNT
			, opts
			, "Unknown joint type: '%s'"
			, type.c_str()
			);

		JointDesc jd;
		jd.type     = jt;
		jd.anchor_0 = sjson::parse_vector3(obj["anchor_0"]);
		jd.anchor_1 = sjson::parse_vector3(obj["anchor_1"]);

		switch (jd.type)
		{
			case JointType::HINGE:
			{
				jd.hinge.use_motor         = sjson::parse_bool (obj["use_motor"]);
				jd.hinge.target_velocity   = sjson::parse_float(obj["target_velocity"]);
				jd.hinge.max_motor_impulse = sjson::parse_float(obj["max_motor_impulse"]);
				jd.hinge.lower_limit       = sjson::parse_float(obj["lower_limit"]);
				jd.hinge.upper_limit       = sjson::parse_float(obj["upper_limit"]);
				jd.hinge.bounciness        = sjson::parse_float(obj["bounciness"]);
				break;
			}
		}

		Buffer buf(default_allocator());
		array::push(buf, (char*)&jd, sizeof(jd));
		return buf;
	}
} // namespace physics_resource

namespace physics_config_resource
{
	void parse_materials(const char* json, Array<PhysicsConfigMaterial>& objects)
	{
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(json, object);

		auto begin = map::begin(object);
		auto end = map::end(object);

		for (; begin != end; ++begin)
		{
			const FixedString key = begin->pair.first;
			const char* value     = begin->pair.second;

			JsonObject material(ta);
			sjson::parse_object(value, material);

			PhysicsConfigMaterial mat;
			mat.name             = StringId32(key.data(), key.length());
			mat.static_friction  = sjson::parse_float(material["static_friction"]);
			mat.dynamic_friction = sjson::parse_float(material["dynamic_friction"]);
			mat.restitution      = sjson::parse_float(material["restitution"]);

			array::push_back(objects, mat);
		}
	}

	void parse_shapes(const char* json, Array<PhysicsConfigShape>& objects)
	{
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(json, object);

		auto begin = map::begin(object);
		auto end = map::end(object);

		for (; begin != end; ++begin)
		{
			const FixedString key = begin->pair.first;
			const char* value     = begin->pair.second;

			JsonObject shape(ta);
			sjson::parse_object(value, shape);

			PhysicsConfigShape ps2;
			ps2.name    = StringId32(key.data(), key.length());
			ps2.trigger = sjson::parse_bool(shape["trigger"]);

			array::push_back(objects, ps2);
		}
	}

	void parse_actors(const char* json, Array<PhysicsConfigActor>& objects)
	{
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(json, object);

		auto begin = map::begin(object);
		auto end = map::end(object);

		for (; begin != end; ++begin)
		{
			const FixedString key = begin->pair.first;
			const char* value     = begin->pair.second;

			JsonObject actor(ta);
			sjson::parse_object(value, actor);

			PhysicsConfigActor pa2;
			pa2.name            = StringId32(key.data(), key.length());
			// pa2.linear_damping  = sjson::parse_float(actor["linear_damping"]);  // 0.0f;
			// pa2.angular_damping = sjson::parse_float(actor["angular_damping"]); // 0.05f;

			const bool has_dynamic         = map::has(actor, FixedString("dynamic"));
			const bool has_kinematic       = map::has(actor, FixedString("kinematic"));
			const bool has_disable_gravity = map::has(actor, FixedString("disable_gravity"));

			pa2.flags = 0;

			if (has_dynamic)
			{
				pa2.flags |= (sjson::parse_bool(actor["dynamic"])
					? 1
					: 0
					);
			}
			if (has_kinematic)
			{
				pa2.flags |= (sjson::parse_bool(actor["kinematic"])
					? PhysicsConfigActor::KINEMATIC
					: 0
					);
			}
			if (has_disable_gravity)
			{
				pa2.flags |= (sjson::parse_bool(actor["disable_gravity"])
					? PhysicsConfigActor::DISABLE_GRAVITY
					: 0
					);
			}

			array::push_back(objects, pa2);
		}
	}

	struct CollisionFilterCompiler
	{
		CompileOptions& _opts;
		Map<StringId32, u32> _filter_map;
		Array<PhysicsCollisionFilter> _filters;
		u32 _filter;

		CollisionFilterCompiler(CompileOptions& opts)
			: _opts(opts)
			, _filter_map(default_allocator())
			, _filters(default_allocator())
			, _filter(1)
		{
		}

		void parse(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject object(ta);
			sjson::parse(json, object);

			auto begin = map::begin(object);
			auto end = map::end(object);
			for (; begin != end; ++begin)
			{
				const FixedString key = begin->pair.first;
				const StringId32 id   = StringId32(key.data(), key.length());

				map::set(_filter_map, id, new_filter_mask());
			}

			begin = map::begin(object);
			end = map::end(object);
			for (; begin != end; ++begin)
			{
				const FixedString key = begin->pair.first;
				const char* value     = begin->pair.second;
				const StringId32 id   = StringId32(key.data(), key.length());

				TempAllocator4096 ta;
				JsonObject filter(ta);
				sjson::parse_object(value, filter);

				JsonArray collides_with(ta);
				sjson::parse_array(filter["collides_with"], collides_with);

				u32 mask = 0;
				for (u32 i = 0; i < array::size(collides_with); ++i)
				{
					const StringId32 fi = sjson::parse_string_id(collides_with[i]);
					mask |= filter_to_mask(fi);
				}

				// Build mask
				PhysicsCollisionFilter pcf;
				pcf.name = id;
				pcf.me   = filter_to_mask(id);
				pcf.mask = mask;

				array::push_back(_filters, pcf);
			}
		}

		u32 new_filter_mask()
		{
			RESOURCE_COMPILER_ASSERT(_filter != 0x80000000u
				, _opts
				, "Too many collision filters"
				);

			const u32 f = _filter;
			_filter = _filter << 1;
			return f;
		}

		u32 filter_to_mask(StringId32 filter)
		{
			RESOURCE_COMPILER_ASSERT(map::has(_filter_map, filter)
				, _opts
				, "Filter not found"
				);

			return map::get(_filter_map, filter, 0u);
		}
	};

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		Array<PhysicsConfigMaterial> materials(default_allocator());
		Array<PhysicsConfigShape> shapes(default_allocator());
		Array<PhysicsConfigActor> actors(default_allocator());
		CollisionFilterCompiler cfc(opts);

		// Parse materials
		if (map::has(object, FixedString("collision_filters")))
			cfc.parse(object["collision_filters"]);
		if (map::has(object, FixedString("materials")))
			parse_materials(object["materials"], materials);
		if (map::has(object, FixedString("shapes")))
			parse_shapes(object["shapes"], shapes);
		if (map::has(object, FixedString("actors")))
			parse_actors(object["actors"], actors);

		// Setup struct for writing
		PhysicsConfigResource pcr;
		pcr.version       = RESOURCE_VERSION_PHYSICS_CONFIG;
		pcr.num_materials = array::size(materials);
		pcr.num_shapes    = array::size(shapes);
		pcr.num_actors    = array::size(actors);
		pcr.num_filters   = array::size(cfc._filters);

		u32 offt = sizeof(PhysicsConfigResource);
		pcr.materials_offset = offt;
		offt += sizeof(PhysicsConfigMaterial) * pcr.num_materials;

		pcr.shapes_offset = offt;
		offt += sizeof(PhysicsConfigShape) * pcr.num_shapes;

		pcr.actors_offset = offt;
		offt += sizeof(PhysicsConfigActor) * pcr.num_actors;

		pcr.filters_offset = offt;
		offt += sizeof(PhysicsCollisionFilter) * pcr.num_filters;

		// Write all
		opts.write(pcr.version);
		opts.write(pcr.num_materials);
		opts.write(pcr.materials_offset);
		opts.write(pcr.num_shapes);
		opts.write(pcr.shapes_offset);
		opts.write(pcr.num_actors);
		opts.write(pcr.actors_offset);
		opts.write(pcr.num_filters);
		opts.write(pcr.filters_offset);

		// Write material objects
		for (u32 i = 0; i < pcr.num_materials; ++i)
		{
			opts.write(materials[i].name._id);
			opts.write(materials[i].static_friction);
			opts.write(materials[i].dynamic_friction);
			opts.write(materials[i].restitution);
		}

		// Write material objects
		for (u32 i = 0; i < pcr.num_shapes; ++i)
		{
			opts.write(shapes[i].name._id);
			opts.write(shapes[i].trigger);
			opts.write(shapes[i]._pad[0]);
			opts.write(shapes[i]._pad[1]);
			opts.write(shapes[i]._pad[2]);
		}

		// Write actor objects
		for (u32 i = 0; i < pcr.num_actors; ++i)
		{
			opts.write(actors[i].name._id);
			opts.write(actors[i].linear_damping);
			opts.write(actors[i].angular_damping);
			opts.write(actors[i].flags);
		}

		for (u32 i = 0; i < array::size(cfc._filters); ++i)
		{
			opts.write(cfc._filters[i].name._id);
			opts.write(cfc._filters[i].me);
			opts.write(cfc._filters[i].mask);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const u32 file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		CE_ASSERT(*(u32*)res == RESOURCE_VERSION_PHYSICS_CONFIG, "Wrong version");
		return res;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	/// Returns the material with the given @a name
	const PhysicsConfigMaterial* material(const PhysicsConfigResource* pcr, StringId32 name)
	{
		const PhysicsConfigMaterial* begin = (PhysicsConfigMaterial*)((const char*)pcr + pcr->materials_offset);
		for (u32 i = 0; i < pcr->num_materials; ++i)
		{
			if (begin[i].name == name)
				return &begin[i];
		}

		CE_FATAL("Material not found");
		return NULL;
	}

	const PhysicsConfigShape* shape(const PhysicsConfigResource* pcr, StringId32 name)
	{
		const PhysicsConfigShape* begin = (PhysicsConfigShape*)((const char*)pcr + pcr->shapes_offset);
		for (u32 i = 0; i < pcr->num_shapes; ++i)
		{
			if (begin[i].name == name)
				return &begin[i];
		}

		CE_FATAL("Shape not found");
		return NULL;
	}

	/// Returns the actor with the given @a name
	const PhysicsConfigActor* actor(const PhysicsConfigResource* pcr, StringId32 name)
	{
		const PhysicsConfigActor* begin = (PhysicsConfigActor*)((const char*)pcr + pcr->actors_offset);
		for (u32 i = 0; i < pcr->num_actors; ++i)
		{
			if (begin[i].name == name)
				return &begin[i];
		}

		CE_FATAL("Actor not found");
		return NULL;
	}

	const PhysicsCollisionFilter* filter(const PhysicsConfigResource* pcr, StringId32 name)
	{
		const PhysicsCollisionFilter* begin = (PhysicsCollisionFilter*)((const char*)pcr + pcr->filters_offset);
		for (u32 i = 0; i < pcr->num_filters; ++i)
		{
			if (begin[i].name == name)
				return &begin[i];
		}

		CE_FATAL("Filter not found");
		return NULL;
	}

} // namespace physics_config_resource
} // namespace crown
