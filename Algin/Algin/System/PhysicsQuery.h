#pragma once
#include "pch.h"

namespace AG::PhysicsQuery
{
    struct Ray
    {
        glm::vec3 origin{};
        glm::vec3 direction{}; // should be normalized
    };

    enum class QueryTriggerInteraction
    {
        UseGlobal,
        Ignore,
        Collide
    };

    struct RaycastHit
    {
        glm::vec3 point{};
        glm::vec3 normal{};
        float distance = 0.0f;


        AG::Component::IComponent* hitComponent = nullptr;

    };
}
