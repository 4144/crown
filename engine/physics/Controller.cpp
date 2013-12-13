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

#include "Controller.h"
#include "Device.h"
#include "MathUtils.h"
#include "PhysicsResource.h"
#include "SceneGraph.h"
#include "Vector3.h"

#include "PxCapsuleController.h"
using physx::PxCapsuleClimbingMode;
using physx::PxCapsuleController;
using physx::PxCapsuleControllerDesc;
using physx::PxControllerFilters;
using physx::PxExtendedVec3;
using physx::PxVec3;

namespace crown
{

//-----------------------------------------------------------------------------
Controller::Controller(const PhysicsResource* pr, SceneGraph& sg, int32_t node, PxScene* scene, PxControllerManager* manager)
	: m_resource(pr)
	, m_scene_graph(sg)
	, m_node(node)
	, m_scene(scene)
	, m_manager(manager)
	, m_controller(NULL)
{
	const PhysicsController contr = pr->controller();

	PxCapsuleControllerDesc desc;
	desc.climbingMode = PxCapsuleClimbingMode::eEASY;
	desc.radius = math::cos(contr.radius);
	desc.height = contr.height;
	desc.slopeLimit = contr.slope_limit;
	desc.stepOffset = contr.step_offset;
	desc.contactOffset = contr.contact_offset;
	desc.upDirection = PxVec3(0.0, 1.0, 0.0);
	desc.material = device()->physx()->createMaterial(0.5f, 0.5f, 0.5f);
	desc.position = PxExtendedVec3(0, 0, 0);
	desc.invisibleWallHeight	= 0.0f;
	desc.maxJumpHeight			= 2.0f;
	desc.scaleCoeff				= 0.9f;

	CE_ASSERT(desc.isValid(), "Capsule is not valid");

	m_controller = manager->createController(*device()->physx(), scene, desc);
	CE_ASSERT(m_controller, "Failed to create controller");
}

//-----------------------------------------------------------------------------
Controller::~Controller()
{
	m_controller->release();
}

//-----------------------------------------------------------------------------
void Controller::move(const Vector3& pos)
{
	PxVec3 disp(pos.x, pos.y, pos.z);
	m_controller->move(disp, 0.01, 1.0 / 60.0, PxControllerFilters());
}

//-----------------------------------------------------------------------------
Vector3 Controller::position() const
{
	PxExtendedVec3 pos = m_controller->getPosition();
	return Vector3(pos.x, pos.y, pos.z);
}

//-----------------------------------------------------------------------------
void Controller::update()
{
	m_scene_graph.set_world_position(m_node, position());
}

} // namespace crown
