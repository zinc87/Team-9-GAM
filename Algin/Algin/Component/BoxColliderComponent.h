#pragma once
#include "../Header/pch.h"
#include "PxPhysicsAPI.h"
#include "../System/PhysXManager.h"

namespace AG {
    namespace Component {

        enum CollisionLayers
        {
            Layer_Default = 1 << 0,
            Layer_Player = 1 << 1,
            Layer_NPC = 1 << 2,
            Layer_Trigger = 1 << 3,
            Layer_Computer = 1 << 4,
            Layer_Centrifuge = 1 << 5,
            Layer_SkinTest = 1 << 6,
            Layer_UrineTest = 1 << 7
        };

        static const char* LayerNames[] = {
            "Default",
            "Player",
            "NPC",
            "Trigger",
            "Computer",
            "Centrifuge",
            "SkinTest",
            "UrineTest"
        };

        static const uint32_t LayerBits[] = {
            Layer_Default,
            Layer_Player,
            Layer_NPC,
            Layer_Trigger,
            Layer_Computer,
            Layer_Centrifuge,
            Layer_SkinTest,
            Layer_UrineTest
        };

        class BoxColliderComponent :
            public IComponent,
            public std::enable_shared_from_this<BoxColliderComponent>
        {
        public:
            using Self = BoxColliderComponent;

            BoxColliderComponent();
            BoxColliderComponent(ID id, ID objId);

            static Type GetStaticType() { return Data::ComponentTypes::BoxCollider; }

            // --- Collision Layers ---
            uint32_t layer = 1 << 0;
            uint32_t mask = 0xFFFFFFFF;

            void Awake() override;
            void Start() override;
            void Update() override;
            void LateUpdate() override;
            void Free() override;
            void Inspector() override;
            bool Serialize(std::ostream& out) const override;
            bool Deserialize(std::istream& in) override;
            std::string GetTypeName() const override { return "AG::Component::BoxColliderComponent"; }

            // Called by PhysX contact callback
            void onCollide() { m_onCollide = true; }
            void notOnCollide() { m_onCollide = false; }

            void SyncTransformToPhysics();
            void SyncShapeToPhysics();

            // Draw helpers
            void SetCaughtVisual(bool isCaught) { m_isCaughtVisual = isCaught; }
            bool m_onCollide = false;
            bool isColliding = false;
            void AssignFrom(const std::shared_ptr<IComponent>& fromCmp) override;
            physx::PxShape* pxShape = nullptr;
            bool needAttach = false;
        REFLECT() {
            return {
                REFLECT_FIELD(center_offset, glm::vec3, "Center Offset"),
                REFLECT_FIELD(halfExtent, glm::vec3, "Half Extent"),
                REFLECT_FIELD(localRotation, glm::vec3, "Local Rotation"),
                REFLECT_FIELD(m_isTrigger, bool, "Is Trigger"),
                REFLECT_FIELD(m_drawCollider, bool, "is Draw Collider"),
                REFLECT_FIELD(m_isCaughtVisual, bool, "Is Caught Visual"),
                REFLECT_FIELD(layer, uint32_t, "Collision Layer"),
                REFLECT_FIELD(ignore_transformRot, bool, "Ignore trfCompRotation")
            };
        }

        public:
            // PhysX handles
            physx::PxRigidActor* pxActor = nullptr;
            physx::PxMaterial* pxMaterial = nullptr;

            // Collider configuration
            glm::vec3 center_offset = glm::vec3(0.0f);
            glm::vec3 halfExtent = glm::vec3(0.5f);
            bool m_isTrigger = false;
            bool m_drawCollider = true;
            bool m_isCaughtVisual = false;

            bool m_ownsActor = false;

            glm::vec3 localRotation = { 0.f, 0.f, 0.f }; // degrees

            bool ignore_transformRot = false;

        private:
			glm::vec3 m_lastHalfExtent = glm::vec3(-1.0f);
			glm::vec3 m_lastCenterOffset = glm::vec3(99999.0f);
            uint32_t  m_lastLayer = 0;
            uint32_t  m_lastMask = 0;
            bool      m_lastIsTrigger = false;

        };

    } // namespace Component
} // namespace AG
