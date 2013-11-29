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

#include "Unit.h"
#include "World.h"
#include "Allocator.h"
#include "Log.h"
#include "UnitResource.h"
#include "SceneGraphManager.h"

namespace crown
{

typedef Id CameraId;
typedef Id SpriteId;

Unit::Unit()
	: m_world(NULL)
	, m_num_cameras(0)
	, m_num_meshes(0)
{
}

//-----------------------------------------------------------------------------
void Unit::create(World& world, SceneGraphManager& sg_mgr, UnitResource* ur, UnitId id, const Vector3& pos, const Quaternion& rot)
{
	// Create the scene graph

	m_scene_graph = sg_mgr.create_scene_graph();

	// Create the root node
	m_root_node = m_scene_graph->create_node(-1, pos, rot);

	// Create renderables
	for (uint32_t i = 0; i < ur->num_renderables(); i++)
	{
		int32_t node = m_scene_graph->create_node(m_root_node, Vector3::ZERO, Quaternion::IDENTITY);

		UnitRenderable renderable = ur->get_renderable(i);

		if (renderable.type == UnitRenderable::MESH)
		{
			MeshId mesh = world.create_mesh(renderable.resource, node, Vector3::ZERO, Quaternion::IDENTITY);
			add_mesh(renderable.name, mesh);
		}
		else if (renderable.type == UnitRenderable::SPRITE)
		{
			SpriteId sprite = world.create_sprite(renderable.resource, node, Vector3::ZERO, Quaternion::IDENTITY);
			world.link_sprite(sprite, id, m_root_node);
			add_sprite(renderable.name, sprite);
		}
		else
		{
			CE_FATAL("Oops, bad renderable type");
		}
	}

	// Create cameras
	for (uint32_t i = 0; i < ur->num_cameras(); i++)
	{
		int32_t cam_node = m_scene_graph->create_node(m_root_node, Vector3::ZERO, Quaternion::IDENTITY);

		UnitCamera camera = ur->get_camera(i);
		CameraId cam = world.create_camera(cam_node, Vector3::ZERO, Quaternion::IDENTITY);
		
		world.link_camera(cam, id, m_root_node);
		add_camera(camera.name, cam);
	}

	m_world = &world;
	m_sg_manager = &sg_mgr;
	m_resource = ur;
	m_id = id;
}

//-----------------------------------------------------------------------------
void Unit::destroy()
{
	// Destroy cameras
	for (uint32_t i = 0; i < m_num_cameras; i++)
	{
		m_world->destroy_camera(m_cameras[i].component);
	}

	// Destroy meshes
	for (uint32_t i = 0; i < m_num_meshes; i++)
	{
		m_world->destroy_mesh(m_meshes[i].component);
	}

	// Destroy sprites
	for (uint32_t i = 0; i < m_num_sprites; i++)
	{
		m_world->destroy_sprite(m_sprites[i].component);
	}

	// Destroy scene graph
	m_sg_manager->destroy_scene_graph(m_scene_graph);
}

//-----------------------------------------------------------------------------
Vector3 Unit::local_position(int32_t node) const
{
	return m_scene_graph->local_position(node);
}

//-----------------------------------------------------------------------------
Quaternion Unit::local_rotation(int32_t node) const
{
	return m_scene_graph->local_rotation(node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Unit::local_pose(int32_t node) const
{
	return m_scene_graph->local_pose(node);
}

//-----------------------------------------------------------------------------
Vector3 Unit::world_position(int32_t node) const
{
	return m_scene_graph->world_position(node);
}

//-----------------------------------------------------------------------------
Quaternion Unit::world_rotation(int32_t node) const
{
	return m_scene_graph->world_rotation(node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Unit::world_pose(int32_t node) const
{
	return m_scene_graph->world_pose(node);
}

//-----------------------------------------------------------------------------
void Unit::set_local_position(const Vector3& pos, int32_t node)
{
	m_scene_graph->set_local_position(node, pos);
}

//-----------------------------------------------------------------------------
void Unit::set_local_rotation(const Quaternion& rot, int32_t node)
{
	m_scene_graph->set_local_rotation(node, rot);
}

//-----------------------------------------------------------------------------
void Unit::set_local_pose(const Matrix4x4& pose, int32_t node)
{
	m_scene_graph->set_local_pose(node, pose);
}

//-----------------------------------------------------------------------------
void Unit::link_node(int32_t child, int32_t parent)
{
	m_scene_graph->link(child, parent);
}

//-----------------------------------------------------------------------------
void Unit::unlink_node(int32_t child)
{
	m_scene_graph->unlink(child);
}

//-----------------------------------------------------------------------------
void Unit::add_component(StringId32 name, Id component, uint32_t& size, Component* array)
{
	Component comp;
	comp.name = name;
	comp.component = component;

	array[size] = comp;
	size++;
}

//-----------------------------------------------------------------------------
Id Unit::find_component(const char* name, uint32_t size, Component* array)
{
	uint32_t name_hash = hash::murmur2_32(name, string::strlen(name), 0);

	Id comp;
	comp.id = INVALID_ID;

	for (uint32_t i = 0; i < size; i++)
	{
		if (name_hash == array[i].name)
		{
			comp = array[i].component;
		}
	}

	return comp;
}

//-----------------------------------------------------------------------------
Id Unit::find_component(uint32_t index, uint32_t size, Component* array)
{
	Id comp;
	comp.id = INVALID_ID;

	if (index < size)
	{
		comp = array[index].component;
	}

	return comp;
}

//-----------------------------------------------------------------------------
void Unit::add_camera(StringId32 name, CameraId camera)
{
	CE_ASSERT(m_num_cameras < MAX_CAMERA_COMPONENTS, "Max camera number reached");

	add_component(name, camera, m_num_cameras, m_cameras);
}

//-----------------------------------------------------------------------------
void Unit::add_mesh(StringId32 name, MeshId mesh)
{
	CE_ASSERT(m_num_meshes < MAX_MESH_COMPONENTS, "Max mesh number reached");

	add_component(name, mesh, m_num_meshes, m_meshes);
}

//-----------------------------------------------------------------------------
void Unit::add_sprite(StringId32 name, SpriteId sprite)
{
	CE_ASSERT(m_num_sprites < MAX_SPRITE_COMPONENTS, "Max sprite number reached");

	add_component(name, sprite, m_num_sprites, m_sprites);
}

//-----------------------------------------------------------------------------
Camera* Unit::camera(const char* name)
{
	CameraId cam = find_component(name, m_num_cameras, m_cameras);

	CE_ASSERT(cam.id != INVALID_ID, "Unit does not have camera with name '%s'", name);

	return m_world->lookup_camera(cam);
}

//-----------------------------------------------------------------------------
Camera* Unit::camera(uint32_t i)
{
	CameraId cam = find_component(i, m_num_cameras, m_cameras);

	CE_ASSERT(cam.id != INVALID_ID, "Unit does not have camera with index '%d'", i);

	return m_world->lookup_camera(cam);
}

//-----------------------------------------------------------------------------
Mesh* Unit::mesh(const char* name)
{
	MeshId mesh = find_component(name, m_num_meshes, m_meshes);

	CE_ASSERT(mesh.id != INVALID_ID, "Unit does not have mesh with name '%s'", name);

	return m_world->lookup_mesh(mesh);
}

//-----------------------------------------------------------------------------
Mesh* Unit::mesh(uint32_t i)
{
	MeshId mesh = find_component(i, m_num_meshes, m_meshes);

	CE_ASSERT(mesh.id != INVALID_ID, "Unit does not have mesh with index '%d'", i);

	return m_world->lookup_mesh(mesh);
}

//-----------------------------------------------------------------------------
Sprite*	Unit::sprite(const char* name)
{
	SpriteId sprite = find_component(name, m_num_sprites, m_sprites);

	CE_ASSERT(sprite.id != INVALID_ID, "Unit does not have sprite with name '%s'", name);

	return m_world->lookup_sprite(sprite);
}

//-----------------------------------------------------------------------------
Sprite*	Unit::sprite(uint32_t i)
{
	SpriteId sprite = find_component(i, m_num_sprites, m_sprites);

	CE_ASSERT(sprite.id != INVALID_ID, "Unit does not have sprite with index '%d'", i);

	return m_world->lookup_sprite(sprite);
}

} // namespace crown
