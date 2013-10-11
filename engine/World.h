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

#include "HeapAllocator.h"
#include "IdTable.h"
#include "LinearAllocator.h"
#include "Unit.h"
#include "Camera.h"
#include "Vector.h"
#include "LinearAllocator.h"
#include "RenderWorld.h"
#include "SoundRenderer.h"

namespace crown
{

#define MAX_UNITS 65000
#define	MAX_SOUNDS 64
#define MAX_CAMERAS 16

typedef Id UnitId;
typedef Id CameraId;
typedef Id MeshId;
typedef Id SoundInstanceId;

struct SoundInstance
{
	SoundId m_sound;
};

class Mesh;
class Sprite;
class Vector3;
class Quaternion;

class World
{
public:
							World();

	void					init();
	void					shutdown();

	UnitId					spawn_unit(const char* name, const Vector3& pos = Vector3::ZERO, const Quaternion& rot = Quaternion(Vector3(0, 1, 0), 0.0f));
	void					kill_unit(UnitId unit);

	void					link_unit(UnitId child, UnitId parent);
	void					unlink_unit(UnitId child, UnitId parent);

	Unit*					lookup_unit(UnitId unit);
	Camera*					lookup_camera(CameraId camera);
	Mesh*					lookup_mesh(MeshId mesh);
	Sprite*					lookup_sprite(SpriteId sprite);

	RenderWorld&			render_world();
	void					update(Camera& camera, float dt);

	CameraId				create_camera(UnitId unit, int32_t node, const Vector3& pos = Vector3::ZERO, const Quaternion& rot = Quaternion::IDENTITY);
	void					destroy_camera(CameraId camera);

	SoundInstanceId			play_sound(const char* name, const bool loop = false, const float volume = 1.0f, const Vector3& pos = Vector3::ZERO, const float range = 50.0f);
	void					pause_sound(SoundInstanceId sound);
	void 					link_sound(SoundInstanceId sound, UnitId unit);
	void					set_listener(const Vector3& pos, const Vector3& vel, const Vector3& or_up, const Vector3& or_at);
	void					set_sound_position(SoundInstanceId sound, const Vector3& pos);
	void					set_sound_range(SoundInstanceId sound, const float range);
	void					set_sound_volume(SoundInstanceId sound, const float vol);
	
private:

	LinearAllocator			m_allocator;
	bool					m_is_init : 1;

	SceneGraph				m_scene_graph[MAX_UNITS];
	ComponentList			m_component[MAX_UNITS];

	IdTable<MAX_UNITS> 		m_unit_table;
	List<Unit>				m_units;

	IdTable<MAX_CAMERAS>	m_camera_table;
	List<Camera>			m_camera;

	IdTable<MAX_SOUNDS> 	m_sound_table;
	SoundInstance			m_sound[MAX_SOUNDS];

	RenderWorld				m_render_world;
};

} // namespace crown
