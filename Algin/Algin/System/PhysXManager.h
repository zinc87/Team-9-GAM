#pragma once
#include "pch.h"
#include <PxPhysicsAPI.h>
#include "PhysicsQuery.h"

namespace AG {
    namespace Component {
        physx::PxFilterFlags AGContactReportFilterShader(
            physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
            physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
            physx::PxPairFlags& pairFlags, const void*, physx::PxU32);

        class BoxColliderComponent;

        struct RaycastHit
        {
            glm::vec3 point;
            glm::vec3 normal;
            float distance;
            System::IObject::ID hitEntityID;
            //void* hitComponent = nullptr;
        };

    }

    namespace Systems {

        class PhysXContactCallback : public physx::PxSimulationEventCallback
        {
        public:
            void onContact(const physx::PxContactPairHeader& pairHeader,
                const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;

            void onTrigger(physx::PxTriggerPair*, physx::PxU32) override {}
            void onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) override {}
            void onWake(physx::PxActor**, physx::PxU32) override {}
            void onSleep(physx::PxActor**, physx::PxU32) override {}
            void onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32) override {}
        };

        class PhysXManager {
        public:
            static PhysXManager* GetInstance();

            void Init();
            void UpdatePhysics(float deltaTime);
            void Shutdown();

            physx::PxPhysics* GetPhysics() const { return mPhysics; }
            physx::PxScene* GetScene() const { return mScene; }
            physx::PxMaterial* GetDefaultMaterial() const { return mDefaultMaterial; }
            bool Raycast(const glm::vec3& origin,const glm::vec3& direction,float maxDistance,AG::Component::RaycastHit& outHit);


            // Optional global like Unity Physics.queriesHitTriggers
            bool queriesHitTriggers = true;

            void RegisterCollider(AG::Component::BoxColliderComponent* c);
            void UnregisterCollider(AG::Component::BoxColliderComponent* c);


        private:
            PhysXManager() = default;
            ~PhysXManager() = default;

            PhysXManager(const PhysXManager&) = delete;
            PhysXManager& operator=(const PhysXManager&) = delete;

            physx::PxDefaultAllocator       mAllocator;
            physx::PxDefaultErrorCallback   mErrorCallback;
            physx::PxFoundation* mFoundation = nullptr;
            physx::PxPhysics* mPhysics = nullptr;
            physx::PxScene* mScene = nullptr;
            physx::PxDefaultCpuDispatcher* mDispatcher = nullptr;
            physx::PxMaterial* mDefaultMaterial = nullptr;
            physx::PxPvd* mPvd = nullptr;
            physx::PxPvdTransport* mTransport = nullptr;
            physx::PxTolerancesScale* mToleranceScale = nullptr;

            PhysXContactCallback mContactCallback;

            std::vector<AG::Component::BoxColliderComponent*> mColliders;
        };
    }
}
