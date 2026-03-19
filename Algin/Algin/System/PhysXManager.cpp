#include "pch.h"
#include "PhysXManager.h"
#include "../Component/BoxColliderComponent.h"


using namespace physx;
using namespace AG::Systems;
using namespace AG::Component;
namespace AG {
    namespace Component {

        // 2. DEFINE the function here.
        physx::PxFilterFlags AGContactReportFilterShader(
            physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
            physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
            physx::PxPairFlags& pairFlags, const void*, physx::PxU32)
        {
            using namespace physx;



            // (The rest of your function body goes here...)
            if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
            {
                pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
                return PxFilterFlag::eDEFAULT;
            }

            if ((filterData0.word0 & filterData1.word1) == 0 ||
                (filterData1.word0 & filterData0.word1) == 0)
            {
                return PxFilterFlag::eSUPPRESS;
            }

            pairFlags = PxPairFlag::eCONTACT_DEFAULT
                | PxPairFlag::eNOTIFY_TOUCH_FOUND
                | PxPairFlag::eNOTIFY_TOUCH_PERSISTS
                | PxPairFlag::eNOTIFY_TOUCH_LOST
                | PxPairFlag::eDETECT_DISCRETE_CONTACT;

            return PxFilterFlag::eDEFAULT;
        }

    } // namespace Component
} // namespace AG

// Static instance
PhysXManager* PhysXManager::GetInstance() {
    static PhysXManager instance;
    return &instance;
}

void PhysXManager::Init() {
    // Foundation
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);
    if (!mFoundation)
        throw std::runtime_error("Failed to create PhysX Foundation");

    // Physics core
    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(), true, mPvd);
    if (!mPhysics)
        throw std::runtime_error("Failed to create PhysX Physics");
    // Scene description
    PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);               //  Gravity

    mDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = mDispatcher;

    sceneDesc.filterShader = AG::Component::AGContactReportFilterShader;

    // Create scene
    mScene = mPhysics->createScene(sceneDesc);
    mScene->setSimulationEventCallback(&mContactCallback);

    if (!mScene)
        throw std::runtime_error("Failed to create PhysX Scene");

    // Verify gravity value
    PxVec3 g = mScene->getGravity();

    // Default material
    mDefaultMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);

}


void PhysXManager::UpdatePhysics(float deltaTime)
{
    if (!mScene) return;

    for (auto* c : mColliders)
    {
        if (!c) continue;
        c->isColliding = false;
        c->m_onCollide = false;
    }

    mScene->simulate(deltaTime);
    mScene->fetchResults(true);
}



void PhysXManager::Shutdown() {

    if (mScene) { mScene->release(); mScene = nullptr; }
    if (mDispatcher) { mDispatcher->release(); mDispatcher = nullptr; }
    if (mDefaultMaterial) { mDefaultMaterial->release(); mDefaultMaterial = nullptr; }
    if (mPhysics) { mPhysics->release(); mPhysics = nullptr; }
    if (mFoundation) { mFoundation->release(); mFoundation = nullptr; }
    if (mPvd)
    {
        mPvd->release();
        mPvd = nullptr;
    }
    if (mTransport)
    {
        mTransport->release();
        mTransport = nullptr;
    }

}

void PhysXContactCallback::onContact(const PxContactPairHeader& pairHeader,
    const PxContactPair* pairs, PxU32 nbPairs)
{
    (void)pairHeader;

    for (physx::PxU32 i = 0; i < nbPairs; i++)
    {
        const physx::PxContactPair& cp = pairs[i];

        auto MarkColliding = [&](physx::PxShape* shape, bool colliding)
            {
                if (!shape) return;
                if (!shape->userData) return;

                auto* col = static_cast<AG::Component::BoxColliderComponent*>(shape->userData);
                if (!col) return;

                if (colliding)
                {
                    col->isColliding = true;
                    col->onCollide();       // sets m_onCollide = true
                }
                else
                {
                    // Optional: only useful if you *don't* clear flags every physics frame
                    col->notOnCollide();    // sets m_onCollide = false
                }
            };

        const bool foundOrPersist =
            (cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) ||
            (cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS);

        if (foundOrPersist)
        {
            MarkColliding(cp.shapes[0], true);
            MarkColliding(cp.shapes[1], true);
        }

        // Optional: handle touch lost
        if (cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            MarkColliding(cp.shapes[0], false);
            MarkColliding(cp.shapes[1], false);
        }
    }
}

bool PhysXManager::Raycast(const glm::vec3& origin,const glm::vec3& direction,float maxDistance,RaycastHit& outHit)
{
    PxVec3 pxOrigin(origin.x, origin.y, origin.z);
    PxVec3 pxDir(direction.x, direction.y, direction.z);

    PxRaycastBuffer hit;
    PxQueryFilterData filterData;
    filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC;

    bool status = mScene->raycast(
        pxOrigin,
        pxDir.getNormalized(),
        maxDistance,
        hit,
        PxHitFlag::ePOSITION | PxHitFlag::eNORMAL,
        filterData
    );

    if (!status || !hit.hasBlock)
        return false;

    const PxRaycastHit& h = hit.block;

    outHit.point = { h.position.x, h.position.y, h.position.z };
    outHit.normal = { h.normal.x, h.normal.y, h.normal.z };
    outHit.distance = h.distance;
    void* data = h.actor->userData;
    if (!data && h.shape) {
        data = h.shape->userData;
    }
       
    BoxColliderComponent* boxColliderComp = static_cast<BoxColliderComponent*>(data);
    outHit.hitEntityID = boxColliderComp->GetObjID();
    return true;
}




void PhysXManager::RegisterCollider(AG::Component::BoxColliderComponent* c)
{
    if (!c) return;
    mColliders.push_back(c);
}

void PhysXManager::UnregisterCollider(AG::Component::BoxColliderComponent* c)
{
    mColliders.erase(std::remove(mColliders.begin(), mColliders.end(), c), mColliders.end());
}




