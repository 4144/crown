/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "math_types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "world_types.h"

namespace crown
{
/// Game level.
///
/// @ingroup World
struct Level
{
	u32 _marker;
	Allocator* _allocator;
	UnitManager* _unit_manager;
	World* _world;
	const LevelResource* _resource;
	Array<UnitId> _unit_lookup;

	///
	Level(Allocator& a, UnitManager& um, World& w, const LevelResource& lr);

	///
	~Level();

	///
	void load(const Vector3& pos, const Quaternion& rot);
};

} // namespace crown
