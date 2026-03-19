#include "pch.h"
#include "RigidBodyComponent.h"
#include "../System/PhysXManager.h"

using namespace physx;
using namespace AG;
using namespace AG::Component;
using namespace AG::Systems; 

void AG::Component::RigidBodyComponent::Awake()
{
}

void AG::Component::RigidBodyComponent::Start()
{
	auto obj = GetObj().lock();
	if (!obj) return;

	auto trf = obj->GetComponent<TransformComponent>();
	if (!trf.lock()) return;

	auto& pos = trf.lock()->GetTransformation().position;
	PxTransform pxTransform(PxVec3(pos.x, pos.y, pos.z));

	auto physics = AG::Systems::PhysXManager::GetInstance()->GetPhysics();
	auto scene = AG::Systems::PhysXManager::GetInstance()->GetScene();

	if (!physics || !scene)
	{
		OutputDebugStringA("[RigidBodyComponent] PhysXManager not initialized!\n");
		return;
	}

	pxMaterial = physics->createMaterial(0.8f, 0.8f, 0.0f);
	pxActor = physics->createRigidDynamic(pxTransform);

	pxActor->setLinearDamping(0.05f);
	pxActor->setAngularDamping(2.0f);

	pxActor->setRigidDynamicLockFlags(
		PxRigidDynamicLockFlag::eLOCK_LINEAR_Y |
		PxRigidDynamicLockFlag::eLOCK_ANGULAR_X |
		PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y |
		PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z
	);

	float mass = (_data.mass <= 0.0f) ? 1.0f : _data.mass;
	PxRigidBodyExt::updateMassAndInertia(*pxActor, mass);

	pxActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
	pxActor->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, false);


	pxActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);


	scene->addActor(*pxActor);
}

void AG::Component::RigidBodyComponent::Update()
{
	auto obj = GetObj().lock();
	if (!obj || !pxActor) return;

	auto trf = obj->GetComponent<TransformComponent>();
	if (!trf.lock()) return;

	// --------------------------------------------------
	// APPLY MOVEMENT FORCE (WASD)
	// --------------------------------------------------

	PxVec3 currentVel = pxActor->getLinearVelocity();

	PxVec3 newVel(
		_data.velocity.x,
		0.0f, // 
		_data.velocity.z
	);

	pxActor->setLinearVelocity(newVel);


	// --------------------------------------------------
	// EXTRA FORCES (jump, knockback, etc.)
	// --------------------------------------------------
	if (glm::length(_data.force) > 0.0001f)
	{
		pxActor->addForce(
			PxVec3(_data.force.x, _data.force.y, _data.force.z),
			PxForceMode::eFORCE
		);
	}

	// --------------------------------------------------
	// SYNC BACK TO ENGINE TRANSFORM
	// --------------------------------------------------
	PxTransform pxT = pxActor->getGlobalPose();
	auto& t = trf.lock()->GetTransformation();
	t.position = glm::vec3(pxT.p.x, pxT.p.y, pxT.p.z);

	// --------------------------------------------------
	// Debug / UI velocity (RENAME VARIABLE)
	// --------------------------------------------------
	PxVec3 physVel = pxActor->getLinearVelocity();
	_data.velocity = glm::vec3(physVel.x, physVel.y, physVel.z);

	// Clear one-frame force
	_data.force = glm::vec3(0.f);
}





void AG::Component::RigidBodyComponent::LateUpdate()
{
}

void AG::Component::RigidBodyComponent::Free()
{
	if (pxActor)
	{
		PhysXManager::GetInstance()->GetScene()->removeActor(*pxActor);
		pxActor->release();
		pxActor = nullptr;
	}
	if (pxMaterial)
	{
		pxMaterial->release();
		pxMaterial = nullptr;
	}
}

void RigidBodyComponent::SetSimulationEnabled(bool enabled)
{
	m_simEnabled = enabled;
	if (!pxActor) return;

	pxActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, !enabled);

	if (enabled) pxActor->wakeUp();
	else pxActor->putToSleep();
}

void RigidBodyComponent::SetFrozen(bool frozen)
{
	m_frozen = frozen;
	if (!pxActor) return;

	if (frozen)
	{
		pxActor->setLinearVelocity(physx::PxVec3(0, 0, 0));
		pxActor->setAngularVelocity(physx::PxVec3(0, 0, 0));
		pxActor->putToSleep();
	}
	else
	{
		pxActor->wakeUp();
	}
}


void AG::Component::RigidBodyComponent::Inspector()
{
	ImGui::DragFloat3("Velocity", &_data.velocity.x, 0.1f);
	ImGui::DragFloat3("Acceleration", &_data.acceleration.x, 0.1f);
	ImGui::DragFloat3("Force", &_data.force.x, 0.1f);
	ImGui::DragFloat("Mass", &_data.mass, 0.1f, 0.1f, 100.0f);
	ImGui::Checkbox("Use Gravity", &_data.useGravity);
	if (_data.useGravity)
	{
		ImGui::DragFloat3("Gravity", &_data.gravity.x, 0.1f);
	}
	ImGui::DragFloat3("Drag", &_data.drag.x, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat3("Final Acceleration", &_data.finalAcceleration.x, 0.1f);

	if (ImGui::Checkbox("Simulation Enabled", &m_simEnabled))
		SetSimulationEnabled(m_simEnabled);

	if (ImGui::Checkbox("Frozen", &m_frozen))
		SetFrozen(m_frozen);

}


void AG::Component::RigidBodyComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
	ASSIGNFROM_FN_BODY(
		this->_data = fromCmpPtr->_data;
	)
}

REGISTER_REFLECTED_TYPE(AG::Component::RigidBodyComponent)