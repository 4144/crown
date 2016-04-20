/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "level.h"
#include "level_resource.h"
#include "unit_manager.h"
#include "unit_resource.h"
#include "world.h"

namespace crown
{
Level::Level(Allocator& a, UnitManager& um, World& w, const LevelResource& lr)
	: _marker(MARKER)
	, _allocator(&a)
	, _unit_manager(&um)
	, _world(&w)
	, _resource(&lr)
	, _unit_lookup(a)
{
}

Level::~Level()
{
	_marker = 0;
}

void Level::load(const Vector3& pos, const Quaternion& rot)
{
	const UnitResource* ur = level_resource::unit_resource(_resource);
	const u32 num_units = ur->num_units;

	array::resize(_unit_lookup, num_units);

	for (u32 i = 0; i < num_units; ++i)
		_unit_lookup[i] = _unit_manager->create();

	spawn_units(*_world, *ur, pos, rot, array::begin(_unit_lookup));

	// Post events
	for (u32 i = 0; i < num_units; ++i)
		_world->post_unit_spawned_event(_unit_lookup[i]);
}

} // namespace crown
