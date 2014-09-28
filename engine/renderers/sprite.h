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

#include "render_world_types.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "quaternion.h"
#include "sprite_resource.h"
#include "material.h"
#include <bgfx.h>

namespace crown
{

class Renderer;
class RenderWorld;
struct SceneGraph;
struct Unit;
typedef Id MaterialId;

//-----------------------------------------------------------------------------
struct Sprite
{
	Sprite(RenderWorld& render_world, SceneGraph& sg, int32_t node, const SpriteResource* sr);

	/// Returns the local position of the sprite.
	Vector3 local_position() const;

	/// Returns the local rotation of the sprite.	
	Quaternion local_rotation() const;

	/// Returns the local pose of the sprite.
	Matrix4x4 local_pose() const;

	/// Returns the world position of the sprite.
	Vector3 world_position() const;

	/// Returns the world rotation of the sprite.
	Quaternion world_rotation() const;

	/// Returns the world pose of the sprite.
	Matrix4x4 world_pose() const;

	/// Sets the local position of the sprite.
	void set_local_position(Unit* unit, const Vector3& pos);

	/// Sets the local rotation of the sprite.
	void set_local_rotation(Unit* unit, const Quaternion& rot);

	/// Sets the local pose of the sprite.
	void set_local_pose(Unit* unit, const Matrix4x4& pose);

	void set_material(MaterialId id);

	void set_frame(uint32_t i);

	void render();

public:

	RenderWorld& m_render_world;
	SceneGraph& m_scene_graph;
	int32_t m_node;
	const SpriteResource* m_resource;
	MaterialId m_material;
	uint32_t m_frame;
};

} // namespace crown
