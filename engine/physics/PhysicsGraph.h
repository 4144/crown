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
#include "Matrix4x4.h"
#include "List.h"

namespace crown
{

struct PhysicsGraph
{
					PhysicsGraph(int32_t index);

	int32_t			create_node(int32_t node, const Vector3& pos, const Quaternion& rot);
	int32_t			create_node(int32_t node, const Matrix4x4& pose);

	Vector3			local_position(int32_t node) const;
	Quaternion		local_rotation(int32_t node) const;
	Matrix4x4		local_pose(int32_t node) const;

	void			set_local_position(int32_t node, const Vector3& pos);
	void			set_local_rotation(int32_t node, const Quaternion& rot);
	void			set_local_pose(int32_t node, const Matrix4x4& pose);

	Vector3			world_position(int32_t node) const;
	Quaternion		world_rotation(int32_t node) const;
	Matrix4x4		world_pose(int32_t node) const;

	void			clear();

	void			update();

public:

	uint32_t		m_index;

	List<Matrix4x4> m_local_poses;
	List<Matrix4x4>	m_world_poses;
	List<uint32_t> 	m_sg_nodes;
};

} // namespace crown