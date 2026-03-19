#pragma once
#include "../Header/pch.h"
#include "PxPhysicsAPI.h"
#include "../System/PhysXManager.h"
#include "IComponent.h"

namespace AG {
    namespace Component {

        class CapsuleColliderComponent :
            public IComponent,
            public std::enable_shared_from_this<CapsuleColliderComponent>
        {
        public:
            CapsuleColliderComponent();
            CapsuleColliderComponent(ID id, ID objId);

            static Type GetStaticType() { return Data::ComponentTypes::CapsuleCollider; }

            // Layers
            uint32_t layer = 1 << 0;
            uint32_t mask = 0xFFFFFFFF;

            // Collider properties
            float radius = 0.5f;
            float halfHeight = 1.0f;
            bool m_isTrigger = false;
            bool m_drawCollider = true;

            // PhysX handles
            physx::PxRigidActor* pxActor = nullptr;
            physx::PxShape* pxShape = nullptr;
            physx::PxMaterial* pxMaterial = nullptr;

            bool needAttach = false;
            bool m_onCollide = false;
            bool isColliding = false;

            // IComponent interface
            void Awake() override;
            void Start() override;
            void Update() override;
            void LateUpdate() override;
            void Free() override;
            void Inspector() override;

            void AssignFrom(const std::shared_ptr<IComponent>& fromCmp) override;
            std::string GetTypeName() const override { return "AG::Component::CapsuleColliderComponent"; }
            using Self = CapsuleColliderComponent;
            REFLECT()
            {
                return {
                    REFLECT_FIELD(radius, float, "Radius"),
                    REFLECT_FIELD(halfHeight, float, "Half Height"),
                    REFLECT_FIELD(m_isTrigger, bool, "Is Trigger"),
                    REFLECT_FIELD(m_drawCollider, bool, "Draw Collider")
                };
            }


            REFLECT_SERIALIZABLE(CapsuleColliderComponent)
        };

    }
}
