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

#include "types.h"
#include "resource_types.h"

#include "PxPhysics.h"
#include "PxScene.h"
#include "PxJoint.h"
#include "PxMaterial.h"

using physx::PxPhysics;
using physx::PxJoint;
using physx::PxScene;
using physx::PxMaterial;

namespace crown
{

struct Actor;

///
/// @ingroup Physics
struct Joint
{
	Joint(PxPhysics* physics, const PhysicsResource* pr, const uint32_t index, const Actor& actor_0, const Actor& actor_1);
	~Joint();

public:

	const PhysicsResource* 	m_resource;
	const uint32_t 			m_index;

	PxJoint* 			m_joint;
	PxMaterial* 		m_mat;
};

} // namespace crown
