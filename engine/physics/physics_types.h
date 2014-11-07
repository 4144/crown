/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "vector3.h"

namespace crown
{

typedef Id ActorId;
typedef Id ControllerId;
typedef Id JointId;
typedef Id RaycastId;

struct Actor;
struct Controller;
struct Joint;
struct Raycast;
class PhysicsWorld;

struct ActorType
{
	enum Enum
	{
		STATIC,
		DYNAMIC_PHYSICAL,
		DYNAMIC_KINEMATIC
	};
};

struct ShapeType
{
	enum Enum
	{
		SPHERE,
		CAPSULE,
		BOX,
		PLANE,
		CONVEX_MESH
	};
};

struct JointType
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

struct CollisionGroup
{
	enum Enum
	{
		GROUP_0		= (1<<0), // Collisions disabled
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

struct CollisionType
{
	enum Enum
	{
		STATIC,
		DYNAMIC,
		BOTH
	};
};

struct CollisionMode
{
	enum Enum
	{
		CLOSEST,
		ANY,
		ALL
	};
};

namespace physics_world
{
struct EventType
{
	enum Enum
	{
		COLLISION,
		TRIGGER
	};
};

struct CollisionEvent
{
	enum Type { BEGIN_TOUCH, END_TOUCH } type;
	Actor* actors[2];
	Vector3 where;
	Vector3 normal;
};

struct TriggerEvent
{
	enum Type { BEGIN_TOUCH, END_TOUCH } type;
	Actor* trigger;
	Actor* other;
};

} // namespace physics_world

} // namespace crown
