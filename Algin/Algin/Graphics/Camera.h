/*****************************************************************//**
 * \file   Camera.h
 * \brief  
 * 
 * \author Brandon
 * \date   September 2025
 *********************************************************************/
#pragma once
#include "pch.h"
enum PROJECTION_TYPE : int
{
	ORTHO = 0,
	PERSPECTIVE = 1
};


struct Camera
{
public:
	/* --- Camera Types--- */
	void makeOrtho() { type = ORTHO; }
	void makePerspective() { type = PERSPECTIVE; }

	/* --- Camera Matrix Updates--- */
	void UpdateView();
	void UpdateProjection();

	/* --- Camera Matrix Getter --- */
	glm::mat4 GetViewMatrix() { return viewMatrix; }
	glm::mat4 GetProjectionMatrix() { return projectionMatrix; }

	glm::mat4 GetInvViewMatrix() { return glm::inverse(viewMatrix); }
	glm::mat4 GetInvProjectionMatrx() { return glm::inverse(projectionMatrix); }

	/* --- Camera Properties Getter/Setter --- */
	glm::vec3&	getCameraPosition() { return position; }
	glm::vec3&	getCameraRotation() { return rotation; }
	float&		getFOV() { return fov; }

	glm::vec3&	getCameraDirection() { return dir; }
	glm::vec3&	getCameraPostiion() { return position; }
	glm::vec3&	getCameraUp() { return up; }
	glm::vec3&	getCameraRight() { return right; }

	float&		getNear() { return nearPlane; }
	float&		getFar() { return farPlane; }
	float&		getVertical() { return vertical_size; }
	float&		getExposure() { return exposure; }

	static bool IsObjectInCamera(glm::mat4 m2w, AABB minmax);

	/* --- Operator Overload --- */
	bool operator==(const Camera& other) const;

	PROJECTION_TYPE type = PERSPECTIVE;
private:
	/* --- Orthographic Size --- */ 
	float vertical_size = 5.f;

	/* --- Camera Position --- */
	glm::vec3 position = { 0.f,3.f,3.f };
	glm::vec3 target = { 0.f,0.f,1.f };
	glm::vec3 dir = glm::vec3(0.000f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(up, dir));

	/* --- Camera Rotation --- */
	glm::vec3 rotation = { 0.f, -90.f, 0.f };

	/* --- Camera Frustum --- */
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
	float fov = 45.f;

	/* --- Camera Matrix --- */
	glm::mat4 viewMatrix{};
	glm::mat4 projectionMatrix{};

	/* --- Post Processes --- */
	float exposure = 1.0f;
};


class CameraManager : public AG::Pattern::ISingleton<CameraManager>
{
public:
	enum CAMERA_TYPE : int
	{
		SCENE_CAMERA,
		GAME_CAMERA
	};

	CameraManager()
	{
		SceneCamera = std::make_shared<Camera>();
		SceneCamera->getFar() = 1000.f;
		CanvasCamera = std::make_shared<Camera>();
	}

	void setGameCamera(std::shared_ptr<Camera> gameCam) { GameCamera = gameCam; }
	std::weak_ptr<Camera> getGameCamera() { return GameCamera; }
	std::shared_ptr<Camera> getSceneCamera() { return SceneCamera; }

	void setCurrentCamera(CAMERA_TYPE type) { CurrentCamera = (type == SCENE_CAMERA) ? SceneCamera : GameCamera; }
	std::weak_ptr<Camera> getCurrentCamera() { return CurrentCamera; }

	// --- Overlay Camera (Picture-in-Picture) ---
	struct OverlayViewport
	{
		// Normalized (0..1), origin at top-left
		float x = 0.02f;
		float y = 0.02f;
		float w = 0.30f;
		float h = 0.30f;
	};

	void setOverlayCamera(std::shared_ptr<Camera> cam) { OverlayCamera = cam; }
	std::weak_ptr<Camera> getOverlayCamera() { return OverlayCamera; }

	void setOverlayActive(bool active) { OverlayActive = active; }
	bool isOverlayActive() const { return OverlayActive; }

	void setOverlayViewport(float x, float y, float w, float h)
	{
		OverlayView = { x, y, w, h };
	}
	OverlayViewport getOverlayViewport() const { return OverlayView; }

private:
	std::shared_ptr<Camera> SceneCamera;
	std::shared_ptr<Camera> CanvasCamera;

	std::weak_ptr<Camera>	GameCamera;
	std::weak_ptr<Camera>	CurrentCamera = SceneCamera;

	std::weak_ptr<Camera>	OverlayCamera;
	bool					OverlayActive = false;
	OverlayViewport			OverlayView{};
};

#define CAMERAMANAGER CameraManager::GetInstance()
