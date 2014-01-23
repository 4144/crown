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

#include "Types.h"
#include "Allocator.h"
#include "File.h"
#include "Bundle.h"
#include "ResourceManager.h"
#include "Vector3.h"

namespace crown
{

//-----------------------------------------------------------------------------
struct PhysicsHeader
{
	uint32_t version;
	uint32_t num_controllers;		// 0 or 1, ATM
	uint32_t controller_offset;
	uint32_t num_actors;
	uint32_t actors_offset;
	uint32_t num_shapes_indices;
	uint32_t shapes_indices_offset;
	uint32_t num_shapes;
	uint32_t shapes_offset;
	uint32_t num_joints;
	uint32_t joints_offset;
};

//-----------------------------------------------------------------------------
struct PhysicsController
{
	StringId32 name;
	float height;			// Height of the capsule
	float radius;			// Radius of the capsule
	float slope_limit;		// The maximum slope which the character can walk up in radians.
	float step_offset;		// Maximum height of an obstacle which the character can climb.
	float contact_offset;	// Skin around the object within which contacts will be generated. Use it to avoid numerical precision issues.
};

//-----------------------------------------------------------------------------
struct PhysicsActorType
{
	enum Enum
	{
		STATIC,
		DYNAMIC_PHYSICAL,
		DYNAMIC_KINEMATIC
	};
};

//-----------------------------------------------------------------------------
struct PhysicsActorGroup
{
	enum Enum
	{
		GROUP_0		= (1<<0),
		GROUP_1		= (1<<1),
		GROUP_2		= (1<<2),
		GROUP_3		= (1<<3),
		GROUP_4		= (1<<4),
		GROUP_5		= (1<<5),
		GROUP_6		= (1<<6),
		GROUP_7		= (1<<7),
		GROUP_8		= (1<<8),
		GROUP_9		= (1<<9),
		GROUP_10	= (1<<10),
		GROUP_11	= (1<<11),
		GROUP_12	= (1<<12),
		GROUP_13	= (1<<13),
		GROUP_14	= (1<<14),
		GROUP_15	= (1<<15),
		GROUP_16	= (1<<16),
		GROUP_17	= (1<<17),
		GROUP_18	= (1<<18),
		GROUP_19	= (1<<19),
		GROUP_20	= (1<<20),
		GROUP_21	= (1<<21),
		GROUP_22	= (1<<22),
		GROUP_23	= (1<<23),
		GROUP_24	= (1<<24),
		GROUP_25	= (1<<25),
		GROUP_26	= (1<<26),
		GROUP_27	= (1<<27),
		GROUP_28	= (1<<28),
		GROUP_29	= (1<<29),
		GROUP_30	= (1<<30),
		GROUP_31	= (1<<31)
	};
};

//-----------------------------------------------------------------------------
struct PhysicsActor
{
	StringId32 name;
	StringId32 node;
	uint32_t type;
	uint32_t group;
	uint32_t mask;
	float static_friction;
	float dynamic_friction;
	float restitution;
	uint32_t num_shapes;
};

//-----------------------------------------------------------------------------
struct PhysicsShapeType
{
	enum Enum
	{
		SPHERE,
		CAPSULE,
		BOX,
		PLANE
	};
};

//-----------------------------------------------------------------------------
struct PhysicsShape
{
	StringId32 name;
	uint32_t type;

	float data_0;
	float data_1;
	float data_2;
	float data_3;
};

//-----------------------------------------------------------------------------
struct PhysicsJointType
{
	enum Enum
	{
		FIXED,
		SPHERICAL,
		REVOLUTE,
		PRISMATIC,
		DISTANCE,
		D6
	};
};

//-----------------------------------------------------------------------------
struct PhysicsJoint
{
	StringId32 name;
	uint32_t type;
	StringId32 actor_0;
	StringId32 actor_1;
	Vector3 anchor_0;
	Vector3 anchor_1;

	bool breakable;
	float break_force;
	float break_torque;

	float restitution;
	float spring;
	float damping;
	float distance;
};

//-----------------------------------------------------------------------------
struct PhysicsResource
{
	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(void* resource)
	{
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT_NOT_NULL(resource);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* resource)
	{
	}

	//-----------------------------------------------------------------------------
	bool has_controller() const
	{
		return ((PhysicsHeader*) this)->num_controllers == 1;
	}

	//-----------------------------------------------------------------------------
	PhysicsController controller() const
	{
		CE_ASSERT(has_controller(), "Controller does not exist");
		const PhysicsHeader* ph = (PhysicsHeader*) this;
		PhysicsController* controller = (PhysicsController*) (((char*) this) + ph->controller_offset);
		return *controller;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_actors() const
	{
		return ((PhysicsHeader*) this)->num_actors;
	}

	//-----------------------------------------------------------------------------
	PhysicsActor actor(uint32_t i) const
	{
		CE_ASSERT(i < num_actors(), "Index out of bounds");
		const PhysicsHeader* ph = (PhysicsHeader*) this;
		PhysicsActor* actor = (PhysicsActor*) (((char*) this) + ph->actors_offset);
		return actor[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_shapes_indices() const
	{
		return ((PhysicsHeader*) this)->num_shapes_indices;
	}

	//-----------------------------------------------------------------------------
	uint32_t shape_index(uint32_t i) const
	{
		CE_ASSERT(i < num_shapes_indices(), "Index out of bounds");

		const PhysicsHeader* ph = (PhysicsHeader*) this;
		uint32_t* index = (uint32_t*) (((char*) this) + ph->shapes_indices_offset);
		return index[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_shapes() const
	{
		return ((PhysicsHeader*) this)->num_shapes;
	}

	//-----------------------------------------------------------------------------
	PhysicsShape shape(uint32_t i) const
	{
		CE_ASSERT(i < num_shapes(), "Index out of bounds");

		const PhysicsHeader* ph = (PhysicsHeader*) this;
		PhysicsShape* shape = (PhysicsShape*) (((char*) this) + ph->shapes_offset);
		return shape[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_joints() const
	{
		return ((PhysicsHeader*) this)->num_joints;
	}

	//-----------------------------------------------------------------------------
	PhysicsJoint joint(uint32_t i) const
	{
		CE_ASSERT(i < num_joints(), "Index out of bounds");

		const PhysicsHeader* ph = (PhysicsHeader*) this;
		PhysicsJoint* joint = (PhysicsJoint*) (((char*) this) + ph->joints_offset);
		return joint[i];		
	}

private:

	// Disable construction
	PhysicsResource();
};

} // namespace crown