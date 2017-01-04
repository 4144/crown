/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "types.h"
#include "world_types.h"

namespace crown
{
/// Manages sound objects in a World.
///
/// @ingroup World
class SoundWorld
{
public:

	virtual ~SoundWorld() {};

	/// Plays the sound @a sr at the given @a volume [0 .. 1].
	/// If loop is true the sound will be played looping.
	virtual SoundInstanceId play(const SoundResource& sr, bool loop, f32 volume, f32 range, const Vector3& pos) = 0;

	/// Stops the sound with the given @a id.
	/// After this call, the instance will be destroyed.
	virtual void stop(SoundInstanceId id) = 0;

	/// Returns whether the sound @a id is playing.
	virtual bool is_playing(SoundInstanceId id) = 0;

	/// Stops all the sounds in the world.
	virtual void stop_all() = 0;

	/// Pauses all the sounds in the world
	virtual void pause_all() = 0;

	/// Resumes all previously paused sounds in the world.
	virtual void resume_all() = 0;

	/// Sets the @a positions (in world space) of @a num sound instances @a ids.
	virtual void set_sound_positions(u32 num, const SoundInstanceId* ids, const Vector3* positions) = 0;

	/// Sets the @a ranges (in meters) of @a num sound instances @a ids.
	virtual void set_sound_ranges(u32 num, const SoundInstanceId* ids, const f32* ranges) = 0;

	/// Sets the @a volumes of @a num sound instances @a ids.
	virtual void set_sound_volumes(u32 num, const SoundInstanceId* ids, const f32* volumes) = 0;

	virtual void reload_sounds(const SoundResource& old_sr, const SoundResource& new_sr) = 0;

	/// Sets the @a pose of the listener in world space.
	virtual void set_listener_pose(const Matrix4x4& pose) = 0;

	virtual void update() = 0;
};

/// Functions to manipulate SoundWorld.
///
/// @ingroup World
namespace sound_world
{
	SoundWorld* create(Allocator& a);
	void destroy(Allocator& a, SoundWorld* sw);
} // namespace sound_world

} // namespace crown
