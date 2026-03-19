#include "pch.h"
#include "BoxColliderComponent.h"
#include "../System/PhysXManager.h"
#include "TransformComponent.h"
#include "RigidBodyComponent.h"


using namespace physx;
using namespace AG;
using namespace AG::Component;
using namespace AG::Systems;

static bool NearlyEqualVec3(const glm::vec3& a, const glm::vec3& b, float epsilon = 0.0001f)
{
    return glm::all(glm::lessThanEqual(glm::abs(a - b), glm::vec3(epsilon)));
}
BoxColliderComponent::BoxColliderComponent()
    : IComponent(Data::ComponentTypes::BoxCollider) {
}

BoxColliderComponent::BoxColliderComponent(ID id, ID objId)
    : IComponent(Data::ComponentTypes::BoxCollider, id, objId) {
}

void BoxColliderComponent::Awake() {}

void AG::Component::BoxColliderComponent::Start()
{
    AG_CORE_INFO("[Collider] Created | Layer:{} Mask:{}", layer, mask);

    auto obj = GetObj().lock();
    if (!obj) return;

    auto trf = obj->GetComponent<TransformComponent>();
    if (trf.expired()) return;

    auto physMgr = AG::Systems::PhysXManager::GetInstance();
    auto physics = physMgr->GetPhysics();
    auto scene = physMgr->GetScene();
    if (!physics || !scene) return;

    // ------------------------------------------------------------
    // Create material + shape FIRST
    // ------------------------------------------------------------
    pxMaterial = physics->createMaterial(0.5f, 0.5f, 0.6f);
    pxShape = physics->createShape(
        physx::PxBoxGeometry(halfExtent.x, halfExtent.y, halfExtent.z),
        *pxMaterial
    );
    pxShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);


    if (!pxShape)
    {
        AG_CORE_ERROR("[Collider] Failed to create PxShape");
        return;
    }

    pxShape->userData = this;

    physMgr->RegisterCollider(this);

    // ------------------------------------------------------------
    // Filter data (Layer/Mask)
    // ------------------------------------------------------------
    physx::PxFilterData filterData;
    filterData.word0 = layer; // this object's layer
    filterData.word1 = mask;  // what layers it collides with
    pxShape->setSimulationFilterData(filterData);
    pxShape->setQueryFilterData(filterData);

    // ------------------------------------------------------------
    // Trigger flags
    // ------------------------------------------------------------
    pxShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, m_isTrigger);
    pxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !m_isTrigger);

    // Optional: if you support offset
    // pxShape->setLocalPose(physx::PxTransform(physx::PxVec3(center_offset.x, center_offset.y, center_offset.z)));

    // ------------------------------------------------------------
    // Actor creation / attachment
    // ------------------------------------------------------------
    auto rb = obj->GetComponent<RigidBodyComponent>();

    // If object has rigidbody but actor not ready yet -> delay attach
    if (!rb.expired() && rb.lock() && !rb.lock()->GetPxActor())
    {
        AG_CORE_WARN("[Collider] Rigidbody exists but PxActor not ready - delaying attach");
        needAttach = true;
        pxActor = nullptr;
        m_ownsActor = false;
        return;
    }

    // If rigidbody exists and actor is ready -> attach shape to dynamic actor
    if (!rb.expired() && rb.lock())
    {
        physx::PxRigidDynamic* dynActor = rb.lock()->GetPxActor();
        if (!dynActor)
        {
            AG_CORE_WARN("[Collider] Rigidbody component exists but PxActor is null - delaying attach");
            needAttach = true;
            pxActor = nullptr;
            m_ownsActor = false;
            return;
        }

        dynActor->attachShape(*pxShape);
        pxActor = dynActor;
        m_ownsActor = false;
        AG_CORE_INFO("[Collider] Attached shape to RigidBody (dynamic)");
    }
    else
    {
        // No rigidbody -> create static actor
        auto& pos = trf.lock()->GetTransformation().position;
        physx::PxTransform pxT(physx::PxVec3(pos.x, pos.y, pos.z));

        pxActor = physics->createRigidStatic(pxT);
        if (!pxActor)
        {
            AG_CORE_ERROR("[Collider] Failed to create PxRigidStatic");
            return;
        }

        pxActor->attachShape(*pxShape);
        scene->addActor(*pxActor);
        m_ownsActor = true;
        AG_CORE_INFO("[Collider] Created STATIC collider");
    }
}


void BoxColliderComponent::Update()
{
    auto obj = GetObj().lock();
    if (!obj) return;

    // Retry dynamic attach if needed
    if (needAttach)
    {
        auto rb = obj->GetComponent<RigidBodyComponent>();  // reuse existing obj

        if (rb.lock() && rb.lock()->GetPxActor() && pxShape)
        {
            physx::PxRigidActor* actor = rb.lock()->GetPxActor();
            actor->attachShape(*pxShape);
            pxActor = actor;
            needAttach = false;

            pxShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);


            AG_CORE_INFO("[Collider] Successfully attached shape on retry");
            pxShape->userData = this;
        }
        SyncTransformToPhysics();
    }

    if (!pxActor) return;

    auto trf = obj->GetComponent<TransformComponent>();
    if (!trf.lock()) return;

    if (!NearlyEqualVec3(halfExtent, m_lastHalfExtent))
    {
        PxBoxGeometry geo(halfExtent.x, halfExtent.y, halfExtent.z);
        pxShape->setGeometry(geo);
        m_lastHalfExtent = halfExtent;
    }

    if (m_isTrigger != m_lastIsTrigger)
    {
        pxShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, m_isTrigger);
        pxShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, !m_isTrigger);
        m_lastIsTrigger = m_isTrigger;
    }

    if (layer != m_lastLayer || mask != m_lastMask)
    {
        PxFilterData fd;
        fd.word0 = layer;
        fd.word1 = mask;
        pxShape->setSimulationFilterData(fd);
        pxShape->setQueryFilterData(fd);

        m_lastLayer = layer;
    }

    if (!NearlyEqualVec3(center_offset, m_lastCenterOffset))
    {
        PxTransform localPose(PxVec3(center_offset.x, center_offset.y, center_offset.z));
        pxShape->setLocalPose(localPose);
        m_lastCenterOffset = center_offset;
    }

    auto rb = obj->GetComponent<RigidBodyComponent>();
    if (rb.expired())
    {
        auto& tf = trf.lock()->GetTransformation();

        PxVec3 p(tf.position.x, tf.position.y, tf.position.z);

        glm::quat q = glm::quat(tf.rotation);
        PxQuat r(q.x, q.y, q.z, q.w);

        if (ignore_transformRot)
        {
			r = PxQuat(0, 0, 0, 1);
        }

        pxActor->setGlobalPose(PxTransform(p, r));
    }
}

void BoxColliderComponent::LateUpdate()
{
    if (!m_drawCollider) return;

    auto obj = GetObj().lock();
    if (!obj) return;

    auto tr = obj->GetComponent<TransformComponent>();
    if (!tr.lock()) return;

    glm::mat4 world = tr.lock()->getM2W();
    glm::vec3 pos = glm::vec3(world[3]);

    glm::vec3 he = halfExtent;

    glm::quat objRot = glm::quat_cast(world);
    if (ignore_transformRot)
    {
        objRot = glm::quat(0, 0, 0, 1);
    }
    glm::quat colRot = glm::quat(glm::radians(localRotation));
    glm::quat finalRot = objRot * colRot;

    glm::mat3 rot = glm::mat3_cast(finalRot);

    glm::vec4 color = isColliding ? glm::vec4(1, 0, 0, 1) : glm::vec4(0, 1, 0, 1);

    glm::vec3 rotatedOffset = rot * center_offset;

    //auto rb = obj->GetComponent<RigidBodyComponent>();
    //if (rb.lock())
    //{
    //    rb.lock()->SetFrozen(isColliding);
    //}

    OBB obb;
    obb.center = pos + rotatedOffset;
    obb.halfExtents = he;
    obb.rotation = rot;

    DebugRenderer::GetInstance().DrawOBBWireframe(obb, color);

}


void BoxColliderComponent::Free()
{
    AG::Systems::PhysXManager::GetInstance()->UnregisterCollider(this);

    // Detach shape only if we own the actor (static collider).
    // When m_ownsActor is false, the RigidBody may have already released the actor.
    if (pxActor && pxShape && m_ownsActor)
    {
        pxActor->detachShape(*pxShape);
    }

    // Only destroy actor if this collider created it (static collider)
    if (pxActor && m_ownsActor)
    {
        auto scene = PhysXManager::GetInstance()->GetScene();
        if (scene) scene->removeActor(*pxActor);
        pxActor->release();
    }

    pxActor = nullptr;
    m_ownsActor = false;

    if (pxShape) { pxShape->release();    pxShape = nullptr; }
    if (pxMaterial) { pxMaterial->release(); pxMaterial = nullptr; }
}


void BoxColliderComponent::Inspector()
{
    ImGui::Text("PhysX Box Collider");

    bool changed = false;
	ImGui::Checkbox("Ignore TRF Rotation", &ignore_transformRot);
    changed |= ImGui::Checkbox("Draw Collider", &m_drawCollider);
    changed |= ImGui::Checkbox("Is Trigger", &m_isTrigger);
    changed |= ImGui::DragFloat3("Half Extents", &halfExtent.x, 0.05f, 0.01f, 10.0f);
    changed |= ImGui::DragFloat3("Center Offset", &center_offset.x, 0.05f, -10.0f, 10.0f);
    changed |= ImGui::DragFloat3("Local Rotation", &localRotation.x, 1.0f, -180.f, 180.f);

    // --- Layer Dropdown ---
    int currentLayerIndex = 0;
    for (int i = 0; i < IM_ARRAYSIZE(LayerBits); i++)
    {
        if (LayerBits[i] == layer)
            currentLayerIndex = i;
    }

    if (ImGui::BeginCombo("Layer", LayerNames[currentLayerIndex]))
    {
        for (int i = 0; i < IM_ARRAYSIZE(LayerNames); i++)
        {
            bool isSelected = (currentLayerIndex == i);
            if (ImGui::Selectable(LayerNames[i], isSelected))
            {
                layer = LayerBits[i];
                changed = true;
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // --- Mask Multi-Select ---
    ImGui::Text("Collision Mask:");
    for (int i = 0; i < IM_ARRAYSIZE(LayerNames); i++)
    {
        bool check = (mask & LayerBits[i]) != 0;
        if (ImGui::Checkbox(LayerNames[i], &check))
        {
            if (check) mask |= LayerBits[i];
            else       mask &= ~LayerBits[i];
            changed = true;
        }
    }

    if (changed && pxShape)
    {
        // Geometry
        physx::PxBoxGeometry geo(halfExtent.x, halfExtent.y, halfExtent.z);
        pxShape->setGeometry(geo);

        // Local offset
        physx::PxTransform localPose(physx::PxVec3(center_offset.x, center_offset.y, center_offset.z));
        pxShape->setLocalPose(localPose);

        // Trigger flags
        pxShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, m_isTrigger);
        pxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !m_isTrigger);

        // Filter
        physx::PxFilterData fd;
        fd.word0 = layer;
        fd.word1 = mask;
        pxShape->setSimulationFilterData(fd);
        pxShape->setQueryFilterData(fd);


        if (pxActor)
        {
            if (auto* dyn = pxActor->is<physx::PxRigidDynamic>())
                dyn->wakeUp();
        }

        // Keep your cached values in sync so Update() won’t “re-apply” weirdly
        m_lastHalfExtent = halfExtent;
        m_lastCenterOffset = center_offset;
        m_lastLayer = layer;
        m_lastMask = mask;
        m_lastIsTrigger = m_isTrigger;
    }

    if (changed)
    {
        SyncShapeToPhysics();

        if (pxActor)
        {
            if (auto* dyn = pxActor->is<physx::PxRigidDynamic>())
                dyn->wakeUp();
        }
    }

}

void BoxColliderComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp)
{
    auto from = std::dynamic_pointer_cast<BoxColliderComponent>(fromCmp);
    if (!from) return;

    this->halfExtent = from->halfExtent;
    this->m_isTrigger = from->m_isTrigger;
    this->m_drawCollider = from->m_drawCollider;
	this->center_offset = from->center_offset;
	this->ignore_transformRot = from->ignore_transformRot;

    this->layer = from->layer;
    this->mask = from->mask;
}

void AG::Component::BoxColliderComponent::SyncTransformToPhysics()
{
    if (!pxActor) return;

    auto obj = GetObj().lock();
    if (!obj) return;

    auto trf = obj->GetComponent<TransformComponent>();
    if (trf.expired()) return;

    auto& tf = trf.lock()->GetTransformation();

    // Convert engine transform → PhysX transform
    physx::PxVec3 p(tf.position.x, tf.position.y, tf.position.z);

    glm::quat q = glm::quat(tf.rotation);  // assuming radians
    physx::PxQuat r(q.x, q.y, q.z, q.w);

    physx::PxTransform pxT(p, r);

    // If this collider owns the actor (static collider)
    if (m_ownsActor)
    {
        pxActor->setGlobalPose(pxT);
    }
    else
    {
        // Only static colliders may be moved by transform
        if (m_ownsActor)
        {
            pxActor->setGlobalPose(pxT);
        }
    }
}

void AG::Component::BoxColliderComponent::SyncShapeToPhysics()
{
    if (!pxShape) return;

    // Update geometry
    physx::PxBoxGeometry geo(halfExtent.x, halfExtent.y, halfExtent.z);
    pxShape->setGeometry(geo);

    // Build local pose (offset + rotation)
    glm::quat q = glm::quat(glm::radians(localRotation));

    physx::PxQuat pxQ(q.x, q.y, q.z, q.w);
    physx::PxVec3 pxP(center_offset.x, center_offset.y, center_offset.z);

    physx::PxTransform localPose(pxP, pxQ);
    pxShape->setLocalPose(localPose);
}



bool BoxColliderComponent::Serialize(std::ostream& out) const { (void)out; return false; }
bool BoxColliderComponent::Deserialize(std::istream& in) { (void)in; return false; }

REGISTER_REFLECTED_TYPE(AG::Component::BoxColliderComponent)
