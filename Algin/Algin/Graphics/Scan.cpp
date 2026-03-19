#include "pch.h"
#include "Scan.h"
#include "../Component/SkinnedMeshRenderer.h"


Ray ScanSetup::BuildRay(float mouseX, float mouseY,
    float viewportW, float viewportH,
    const glm::mat4& view,
    const glm::mat4& proj,
    const glm::vec3& camPos)
{
    // 1. Mouse -> NDC
    float x = 2.0f * mouseX / viewportW - 1.0f;
    float y = 2.0f * mouseY / viewportH - 1.0f;

    // 2. NDC -> eye space (z = -1 on near plane, w = 1)
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    glm::mat4 invProj = glm::inverse(proj);
    glm::vec4 rayEye = invProj * rayClip;

    // Put it on the view-space ray direction (z = -1, w = 0)
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // 3. Eye -> world
    glm::mat4 invView = glm::inverse(view);
    glm::vec4 rayWorld4 = invView * rayEye;
    glm::vec3 rayDir = glm::normalize(glm::vec3(rayWorld4));

    Ray ray;
    ray.origin = camPos;
    ray.dir = rayDir;
    return ray;
}

bool ScanSetup::RayAABB(const Ray& ray, const glm::vec3& bmin, const glm::vec3& bmax, float& tHit)
{
    float tmin = (bmin.x - ray.origin.x) / ray.dir.x;
    float tmax = (bmax.x - ray.origin.x) / ray.dir.x;
    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (bmin.y - ray.origin.y) / ray.dir.y;
    float tymax = (bmax.y - ray.origin.y) / ray.dir.y;
    if (tymin > tymax) std::swap(tymin, tymax);

    // X/Y overlap?
    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    float tzmin = (bmin.z - ray.origin.z) / ray.dir.z;
    float tzmax = (bmax.z - ray.origin.z) / ray.dir.z;
    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    // X/Y/Z overlap?
    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;

    // We want the first hit in front of the camera
    if (tmax < 0.0f)
        return false;

    tHit = (tmin >= 0.0f) ? tmin : tmax;
    return true;
}


bool ScanSetup::ScanHit(const Ray& ray, glm::vec3& outHitPos, float& outRef
    , std::vector<SubMeshSkinned>& gSubMeshes, const glm::mat4& modelMat)
{
    float closestT = std::numeric_limits<float>::max();
    bool hitSomething = false;
    AABB worldBox;
    worldBox.min = glm::vec3(1e30f);
    worldBox.max = glm::vec3(-1e30f);



    for (auto& subMesh : gSubMeshes)
    {
        AABB subWorld = subMesh.minmax.transform(modelMat);

        worldBox.min = glm::min(worldBox.min, subWorld.min);
        worldBox.max = glm::max(worldBox.max, subWorld.max);

    }

    float tHit;
    if (RayAABB(ray, worldBox.min, worldBox.max, tHit))
    {
        if (tHit < closestT)
        {
            closestT = tHit;
            hitSomething = true;
        }
    }


    if (hitSomething)
    {
        outHitPos = ray.origin + ray.dir * closestT;
        glm::vec3 worldSize = worldBox.max - worldBox.min;

        float characterMaxExtent = glm::max(worldSize.x, glm::max(worldSize.y, worldSize.z));
        float characterHalfHeight = 0.5f * characterMaxExtent;

        // Use this as reference for scan radius
        outRef = characterHalfHeight;
    }

    return hitSomething;
}
