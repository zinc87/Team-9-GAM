
/*****************************************************************//**
 * \file   Camera.cpp
 * \brief  
 * 
 * \author Brandon
 * \date   September 2025
 *********************************************************************/
#include "pch.h"
#include "Camera.h"

void Camera::UpdateView()
{
    // Compute the front vector from yaw and pitch
    glm::vec3 newDir;
    newDir.x = cos(glm::radians(rotation[1])) * cos(glm::radians(rotation[0]));
    newDir.y = sin(glm::radians(rotation[0]));
    newDir.z = sin(glm::radians(rotation[1])) * cos(glm::radians(rotation[0]));
    dir = glm::normalize(newDir);

    // Recalculate right and up vectors
    right = glm::normalize(glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)));
    up = glm::normalize(glm::cross(right, dir));

    // Update the view matrix using position and dir
    viewMatrix = glm::lookAt(position, position + dir, up);
}

void Camera::UpdateProjection()
{
    std::pair<int, int> AR = AGWINDOW.getAspectRatio();

    if (AR.second == 0)
        return;

    float aspectRatio = static_cast<float>(AR.first) / static_cast<float>(AR.second);


    if (type == PERSPECTIVE)
    {
        float fovD = glm::radians(fov);

        projectionMatrix = glm::perspective(fovD, aspectRatio, nearPlane, farPlane);
    }
    else
    {
        const float halfH = vertical_size * 0.5f;
        const float halfW = halfH * aspectRatio;

        // OpenGL-style ortho (NDC z in [-1, 1])
        projectionMatrix = glm::ortho(
            -halfW, halfW,   // left, right
            -halfH, halfH,   // bottom, top
            nearPlane, farPlane
        );
    }
}

bool Camera::operator==(const Camera& other) const {
    return position == other.position &&
        target == other.target &&
        dir == other.dir &&
        up == other.up &&
        right == other.right &&
        rotation == other.rotation &&
        nearPlane == other.nearPlane &&
        farPlane == other.farPlane &&
        fov == other.fov &&
        vertical_size == other.vertical_size &&
        type == other.type;
}

bool Camera::IsObjectInCamera(glm::mat4 m2w, AABB minmax)
{
    // === Step 1: Transform AABB to world space ===
    glm::vec3 worldMin = glm::vec3(m2w * glm::vec4(minmax.min, 1.0f));
    glm::vec3 worldMax = glm::vec3(m2w * glm::vec4(minmax.max, 1.0f));

    // === Step 2: Compute combined ViewProjection matrix ===
    Camera& cam = *CAMERAMANAGER.getCurrentCamera().lock();
    glm::mat4 VP = cam.GetProjectionMatrix() * cam.GetViewMatrix();

    // === Step 3: Extract frustum planes from VP matrix ===
    // Each plane is in the form (a,b,c,d): ax + by + cz + d = 0
    glm::vec4 planes[6];

    // Left
    planes[0] = glm::vec4(
        VP[0][3] + VP[0][0],
        VP[1][3] + VP[1][0],
        VP[2][3] + VP[2][0],
        VP[3][3] + VP[3][0]);

    // Right
    planes[1] = glm::vec4(
        VP[0][3] - VP[0][0],
        VP[1][3] - VP[1][0],
        VP[2][3] - VP[2][0],
        VP[3][3] - VP[3][0]);

    // Bottom
    planes[2] = glm::vec4(
        VP[0][3] + VP[0][1],
        VP[1][3] + VP[1][1],
        VP[2][3] + VP[2][1],
        VP[3][3] + VP[3][1]);

    // Top
    planes[3] = glm::vec4(
        VP[0][3] - VP[0][1],
        VP[1][3] - VP[1][1],
        VP[2][3] - VP[2][1],
        VP[3][3] - VP[3][1]);

    // Near
    planes[4] = glm::vec4(
        VP[0][3] + VP[0][2],
        VP[1][3] + VP[1][2],
        VP[2][3] + VP[2][2],
        VP[3][3] + VP[3][2]);

    // Far
    planes[5] = glm::vec4(
        VP[0][3] - VP[0][2],
        VP[1][3] - VP[1][2],
        VP[2][3] - VP[2][2],
        VP[3][3] - VP[3][2]);

    // === Step 4: Normalize plane equations ===
    for (int i = 0; i < 6; i++)
    {
        float len = glm::length(glm::vec3(planes[i]));
        if (len > 0.0001f)
            planes[i] /= len;
    }

    // === Step 5: Test AABB against all frustum planes ===
    // If AABB is fully outside any plane, it's culled.
    for (int i = 0; i < 6; i++)
    {
        const glm::vec3 normal = glm::vec3(planes[i]);
        float d = planes[i].w;

        // Positive vertex (the farthest point along the plane normal)
        glm::vec3 positive = worldMin;
        if (normal.x >= 0) positive.x = worldMax.x;
        if (normal.y >= 0) positive.y = worldMax.y;
        if (normal.z >= 0) positive.z = worldMax.z;

        // Plane–point distance
        float dist = glm::dot(normal, positive) + d;

        // If the positive vertex is outside, the entire box is outside
        if (dist < 0)
            return false;
    }

    // Otherwise, at least part of the box is inside
    return true;
}

