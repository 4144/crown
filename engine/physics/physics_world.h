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

#include "id_array.h"
#include "pool_allocator.h"
#include "physics_types.h"
#include "physics_callback.h"
#include "event_stream.h"
#include "physics_types.h"
#include "math_types.h"
#include "world_types.h"
#include "PxScene.h"
#include "PxCooking.h"
#include "PxDefaultCpuDispatcher.h"
#include "PxControllerManager.h"

#include "log.h"

using physx::PxControllerManager;
using physx::PxScene;
using physx::PxDefaultCpuDispatcher;
using physx::PxActor;
using physx::PxOverlapHit;
using physx::PxOverlapBuffer;
using physx::PxMaterial;
using physx::PxShape;
using physx::PxPhysics;
using physx::PxCooking;

namespace crown
{

struct SceneGraph;
class World;
struct PhysicsResource;
struct PhysicsConfigResource;
struct Unit;
struct DebugLine;

/// Manages physics objects in a World.
///
/// @ingroup Physics
class PhysicsWorld
{
public:

	PhysicsWorld(World& world);
	~PhysicsWorld();

	ActorId create_actor(const PhysicsResource* res, const uint32_t index, SceneGraph& sg, int32_t node, UnitId unit_id);
	void destroy_actor(ActorId id);

	ControllerId create_controller(const PhysicsResource* pr, SceneGraph& sg, int32_t node);
	void destroy_controller(ControllerId id);

	JointId create_joint(const PhysicsResource* pr, const uint32_t index, const Actor& actor_0, const Actor& actor_1);
	void destroy_joint(JointId id);

	RaycastId create_raycast(CollisionMode::Enum mode, CollisionType::Enum filter);
	void destroy_raycast(RaycastId id);

	/// Returns the gravity.
	Vector3 gravity() const;

	/// Sets the gravity.
	void set_gravity(const Vector3& g);

	/// Finds all actors in the physics world that are in a particular shape (supported: spheres, capsules and boxes)
	void overlap_test(CollisionType::Enum filter, ShapeType::Enum type,
						const Vector3& pos, const Quaternion& rot, const Vector3& size, Array<Actor*>& actors);

	/// Updates the physics simulation.
	void update(float dt);

	/// Draws debug lines.
	void draw_debug();

	Actor* get_actor(ActorId id);
	Controller* get_controller(ControllerId id);
	Raycast* get_raycast(RaycastId id);

	World& world() { return m_world; }
	EventStream& events() { return m_events; }

public:

	PxPhysics* physx_physics();
	PxCooking* physx_cooking();
	PxScene* physx_scene();
	const PhysicsConfigResource* resource() { return m_resource; }

private:

	World& m_world;
	PxControllerManager* m_controller_manager;
	PxScene* m_scene;
	PxDefaultCpuDispatcher* m_cpu_dispatcher;

	PxOverlapHit m_hits[64]; // hardcoded
	PxOverlapBuffer m_buffer;

	PoolAllocator m_actors_pool;
	PoolAllocator m_controllers_pool;
	PoolAllocator m_joints_pool;
	PoolAllocator m_raycasts_pool;

	IdArray<CE_MAX_ACTORS, Actor*>	m_actors;
	IdArray<CE_MAX_CONTROLLERS, Controller*> m_controllers;
	IdArray<CE_MAX_JOINTS, Joint*> m_joints;
	IdArray<CE_MAX_RAYCASTS, Raycast*> m_raycasts;

	// Events management
	EventStream m_events;
	PhysicsSimulationCallback m_callback;

	const PhysicsConfigResource* m_resource;

	#if defined(CROWN_DEBUG)
		DebugLine* m_debug_line;
	#endif
};

} // namespace crown
