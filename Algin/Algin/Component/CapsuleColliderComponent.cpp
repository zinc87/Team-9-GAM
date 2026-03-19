#include "pch.h"
#include "CapsuleColliderComponent.h"
#include "TransformComponent.h"
#include "RigidBodyComponent.h"
#include "../System/PhysXManager.h"

using namespace physx;
using namespace AG;
using namespace AG::Component;

CapsuleColliderComponent::CapsuleColliderComponent()
    : IComponent(Data::ComponentTypes::CapsuleCollider) {
}

CapsuleColliderComponent::CapsuleColliderComponent(ID id, ID objId)
    : IComponent(Data::ComponentTypes::CapsuleCollider, id, objId) {
}

void CapsuleColliderComponent::Awake() {}

void CapsuleColliderComponent::Start()
{
    auto obj = GetObj().lock();
    if (!obj) return;

    auto tr = obj->GetComponent<TransformComponent>();
    if (!tr.lock()) return;

    auto physics = Systems::PhysXManager::GetInstance()->GetPhysics();
    auto scene = Systems::PhysXManager::GetInstance()->GetScene();
    if (!physics || !scene) return;

    // Material + Shape
    pxMaterial = physics->createMaterial(0.5f, 0.5f, 0.6f);
    pxShape = physics->createShape(
        PxCapsuleGeometry(radius, halfHeight),
        *pxMaterial
    );

    // Layer + mask
    PxFilterData f;
    f.word0 = layer;
    f.word1 = mask;

    pxShape->setSimulationFilterData(f);
    pxShape->setQueryFilterData(f);

    pxShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, m_isTrigger);
    pxShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, !m_isTrigger);

    auto& pos = tr.lock()->GetTransformation().position;
    PxTransform T(PxVec3(pos.x, pos.y, pos.z));

    auto rb = obj->GetComponent<RigidBodyComponent>();

    if (rb.lock() && !rb.lock()->GetPxActor())
    {
        needAttach = true;
        return;
    }

    if (rb.lock())
    {
        auto dyn = rb.lock()->GetPxActor();
        dyn->attachShape(*pxShape);
        pxActor = dyn;
    }
    else
    {
        pxActor = physics->createRigidStatic(T);
        pxActor->attachShape(*pxShape);
        scene->addActor(*pxActor);
    }

    pxShape->userData = this;
}

void CapsuleColliderComponent::Update()
{
    if (needAttach)
    {
        auto obj = GetObj().lock();
        if (!obj) return;

        auto rb = obj->GetComponent<RigidBodyComponent>();
        if (rb.lock() && rb.lock()->GetPxActor() && pxShape)
        {
            auto actor = rb.lock()->GetPxActor();
            actor->attachShape(*pxShape);
            pxActor = actor;
            needAttach = false;
        }
    }

    isColliding = false;
}

void CapsuleColliderComponent::LateUpdate()
{
    if (!m_drawCollider) return;

    auto obj = GetObj().lock();
    if (!obj) return;

    auto tr = obj->GetComponent<TransformComponent>();
    if (!tr.lock()) return;

    auto& tf = tr.lock()->GetTransformation();

    // Build quaternion from Euler
    glm::quat q = glm::quat(tf.rotation);

    // Convert quaternion to rotation matrix
    glm::mat3 rot = glm::mat3_cast(q);

    // Capsule axis BEFORE rotation (pointing up)
    glm::vec3 localUp = glm::vec3(0, 1, 0);

    // Capsule axis AFTER rotation
    glm::vec3 axis = rot * localUp;

    // Compute capsule endpoints
    glm::vec3 top = tf.position + axis * halfHeight;
    glm::vec3 bottom = tf.position - axis * halfHeight;

    // Draw rotated capsule
    glm::vec4 color = isColliding ? glm::vec4(1, 0, 0, 1) : glm::vec4(0, 1, 0, 1);

    DebugRenderer::GetInstance().DrawCapsuleWireframe(top, bottom, radius, color);

}

void CapsuleColliderComponent::Free()
{
    if (pxActor)
    {
        auto scene = Systems::PhysXManager::GetInstance()->GetScene();
        if (scene) scene->removeActor(*pxActor);
        pxActor->release();
        pxActor = nullptr;
    }
    if (pxShape) { pxShape->release(); pxShape = nullptr; }
    if (pxMaterial) { pxMaterial->release(); pxMaterial = nullptr; }
}

void CapsuleColliderComponent::Inspector()
{
    ImGui::Text("Capsule Collider");
    ImGui::Checkbox("Draw Collider", &m_drawCollider);
    ImGui::Checkbox("Is Trigger", &m_isTrigger);
    ImGui::DragFloat("Radius", &radius, 0.05f, 0.01f, 10.0f);
    ImGui::DragFloat("Half Height", &halfHeight, 0.05f, 0.01f, 10.0f);

    ImGui::Text("Layer");
    int curIdx = 0;
    for (int i = 0; i < IM_ARRAYSIZE(LayerBits); i++)
        if (LayerBits[i] == layer)
            curIdx = i;

    if (ImGui::BeginCombo("Layer", LayerNames[curIdx]))
    {
        for (int i = 0; i < IM_ARRAYSIZE(LayerNames); i++)
        {
            bool sel = curIdx == i;
            if (ImGui::Selectable(LayerNames[i], sel))
                layer = LayerBits[i];
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Text("Collision Mask");
    for (int i = 0; i < IM_ARRAYSIZE(LayerNames); i++)
    {
        bool checked = (mask & LayerBits[i]) != 0;
        if (ImGui::Checkbox(LayerNames[i], &checked))
        {
            if (checked) mask |= LayerBits[i];
            else         mask &= ~LayerBits[i];
        }
    }
}

void CapsuleColliderComponent::AssignFrom(const std::shared_ptr<IComponent>& from)
{
    auto c = std::dynamic_pointer_cast<CapsuleColliderComponent>(from);
    if (!c) return;

    radius = c->radius;
    halfHeight = c->halfHeight;
    m_isTrigger = c->m_isTrigger;
    m_drawCollider = c->m_drawCollider;
    layer = c->layer;
    mask = c->mask;
}

REGISTER_REFLECTED_TYPE(AG::Component::CapsuleColliderComponent)
