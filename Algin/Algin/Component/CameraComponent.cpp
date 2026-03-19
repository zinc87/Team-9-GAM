#include "pch.h"
#include "CameraComponent.h"

void AG::Component::CameraComponent::SyncFromCamera()
{
    if (!m_camera) return;
    reflected_near = m_camera->getNear();
    reflected_far = m_camera->getFar();
    reflected_fov = m_camera->getFOV();
    reflected_vertical = m_camera->getVertical();
    reflected_proj = static_cast<int>(m_camera->type);
    reflected_isGameCam = (CAMERAMANAGER.getGameCamera().lock() == m_camera);
}

void AG::Component::CameraComponent::SyncToCamera()
{
    if (!m_camera) return;

    // set projection 
    if (reflected_proj == static_cast<int>(PROJECTION_TYPE::ORTHO)) m_camera->makeOrtho();
    else m_camera->makePerspective();

    m_camera->getNear() = reflected_near;
    m_camera->getFar() = reflected_far;
    m_camera->getFOV() = reflected_fov;
    m_camera->getVertical() = reflected_vertical;

    // Game camera management: allow both set AND clear
    if (reflected_isGameCam) {
        // user checked the box => set this camera as the game camera
        CAMERAMANAGER.setGameCamera(m_camera);
    }
    else {
        // user unchecked the box => if manager currently points to this camera, clear it
        if (CAMERAMANAGER.getGameCamera().lock() == m_camera) {
            CAMERAMANAGER.setGameCamera(nullptr); // clear game camera selection
        }
    }
}

void AG::Component::CameraComponent::Awake()
{
	//m_camera = std::make_shared<Camera>();

 //   // initialize reflected values from camera
	SyncToCamera();

}

void AG::Component::CameraComponent::Start()
{
}

void AG::Component::CameraComponent::Update()
{
	auto obj = GetObj().lock();
	if (obj == nullptr) return;

	if (reflected_isGameCam || CAMERAMANAGER.getGameCamera().lock() == m_camera) 
	{
		CAMERAMANAGER.setGameCamera(m_camera);
	}

	auto trf_weak = GetObj().lock()->GetComponent<Component::TransformComponent>();
	auto trf_shared = trf_weak.lock();
	if (trf_shared)
	{
		m_camera->getCameraPostiion()	= trf_shared->GetPosiion();
		m_camera->getCameraRotation() = trf_shared->GetRot();
		m_camera->getCameraRotation().y -= 90.f;
	}

	m_camera->UpdateView();
	m_camera->UpdateProjection();
}

void AG::Component::CameraComponent::LateUpdate()
{
	if (PIPELINE.GetPipeline() == RenderPipeline::LIGHT)
	{
		// should be in LateUpdate();
		if (CAMERAMANAGER.getCurrentCamera().lock() == CAMERAMANAGER.getGameCamera().lock())
			return;

		DebugRenderer::GetInstance().DrawWireframe(m_camera->GetProjectionMatrix(), m_camera->GetViewMatrix());

		if (drawDebugRay)
		{
			auto ray = GetForwardRay();

			// Offset so it doesn't hide inside frustum lines
			glm::vec3 start = ray.origin + ray.direction * 0.2f;
			float maxDistance = debugRayLength;

			AG::Component::RaycastHit hit{};
			bool hasHit = AG::Systems::PhysXManager::GetInstance()->Raycast(start,ray.direction,maxDistance,hit);

			//if (hasHit && hit.hitComponent)
			//{
			//	auto* collider =
			//		static_cast<AG::Component::BoxColliderComponent*>(hit.hitComponent);

			//	if (collider)
			//	{
			//		using namespace AG::Component;

			//		if (collider->layer & Layer_Default)
			//			AG_CORE_INFO("[Raycast] Hit DEFAULT box");

			//		if (collider->layer & Layer_Player)
			//			AG_CORE_INFO("[Raycast] Hit PLAYER box");

			//		if (collider->layer & Layer_Enemy)
			//			AG_CORE_INFO("[Raycast] Hit ENEMY box");

			//		if (collider->layer & Layer_Trigger)
			//			AG_CORE_INFO("[Raycast] Hit TRIGGER box");
			//	}
			//}



			// Decide color based on hit
			glm::vec4 color = hasHit
				? glm::vec4(1, 0, 0, 1)   //  RED when hit
				: glm::vec4(0, 1, 0, 1);  //  GREEN when no hit

			// End point
			glm::vec3 end = hasHit
				? hit.point
				: start + ray.direction * maxDistance;

			DebugRenderer::GetInstance().DrawLine(start, end, color);
		}

	}

}

void AG::Component::CameraComponent::Free()
{
}

void AG::Component::CameraComponent::Inspector()
{

	if (m_camera.get() == nullptr) return;
	ImGui::Text("Camera Debug");
	ImGui::Checkbox("Draw Debug Ray", &drawDebugRay);
	if (drawDebugRay)
	{
		ImGui::DragFloat("Ray Length",&debugRayLength,
			0.5f,      // speed
			0.1f,      // min
			500.0f     // max
	);
	}
	/*float x_offset = ImGui::GetContentRegionAvail().x * 0.3f;
	float x_start = ImGui::GetCursorPos().x;
	float x_width = ImGui::GetContentRegionAvail().x * 0.7f;

	std::vector<const char*> projectionList = { "Orthographic", "Perspective" };
	int currentProj = m_camera->type;
	ImGui::Text("Projection"); ImGui::SameLine();

	ImGui::SetCursorPosX(x_start + x_offset);
	ImGui::SetNextItemWidth(x_width);
	if (ImGui::Combo("##PL", &currentProj, projectionList.data(), projectionList.size()))
	{
		switch (currentProj)
		{
		case ORTHO:
			m_camera->makeOrtho();
			break;
		case PERSPECTIVE:
			m_camera->makePerspective();
			break;
		}
	}


	if (m_camera->type == PROJECTION_TYPE::PERSPECTIVE)
	{
		ImGui::Text("Near Plane"); ImGui::SameLine();
		ImGui::SetCursorPosX(x_start + x_offset);
		ImGui::SetNextItemWidth(x_width);
		ImGui::InputFloat("##NearPlane", &m_camera->getNear());

		ImGui::Text("Far Plane"); ImGui::SameLine();
		ImGui::SetCursorPosX(x_start + x_offset);
		ImGui::SetNextItemWidth(x_width);
		ImGui::InputFloat("##FarPlane", &m_camera->getFar());

		ImGui::Text("FOV"); ImGui::SameLine();
		ImGui::SetCursorPosX(x_start + x_offset);
		ImGui::SetNextItemWidth(x_width);
		ImGui::InputFloat("##FOV", &m_camera->getFOV());
	}
	else
	{
		ImGui::Text("Vertical Size"); ImGui::SameLine();
		ImGui::SetCursorPosX(x_start + x_offset);
		ImGui::SetNextItemWidth(x_width);
		ImGui::InputFloat("##VS", &m_camera->getVertical());
	}

	bool isGameCam = false;
	if (CAMERAMANAGER.getGameCamera().lock() == m_camera)
	{
		isGameCam = true;
	}
	if (ImGui::Checkbox("Game Camera##ONLY1", &isGameCam))
	{
		CAMERAMANAGER.setGameCamera(m_camera);
	}*/
	SyncToCamera();

    AG::RenderFieldsInspector(this, Self::GetFields());

    // After user edits the mirrors, push changes back to camera
    SyncToCamera();
}

AG::Component::CameraRay
AG::Component::CameraComponent::GetForwardRay()
{
	AG::Component::CameraRay ray{};

	auto obj = GetObj().lock();
	if (!obj) return ray;

	auto transformWeak = obj->GetComponent<TransformComponent>();
	auto transform = transformWeak.lock();
	if (!transform) return ray;

	ray.origin = transform->GetPosiion();

	glm::vec3 forward = glm::vec3(transform->GetForward());
	ray.direction = glm::normalize(-forward);

	return ray;
}

void AG::Component::CameraComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
	ASSIGNFROM_FN_BODY(
		this->reflected_far = fromCmpPtr->reflected_far;
		this->reflected_fov = fromCmpPtr->reflected_fov;
		this->reflected_isGameCam = fromCmpPtr->reflected_isGameCam;
		this->reflected_near = fromCmpPtr->reflected_near;
		this->reflected_proj = fromCmpPtr->reflected_proj;
		this->reflected_vertical = fromCmpPtr->reflected_vertical;
	)
}

bool AG::Component::CameraComponent::Serialize(std::ostream& out) const {
	auto writeBlock = [&](uint32_t hash, const void* data, uint32_t size) -> bool {
		return IO::WriteUInt32(out, hash)
			&& IO::WriteUInt32(out, size)
			&& IO::WriteBytes(out, data, size);
		};

	if (!writeBlock(CAMERA_TYPE_HASH, &reflected_proj, sizeof(reflected_proj)))     return false;
	if (!writeBlock(CAMERA_NEAR_HASH, &reflected_near, sizeof(reflected_near)))     return false;
	if (!writeBlock(CAMERA_FAR_HASH, &reflected_far, sizeof(reflected_far)))      return false;
	if (!writeBlock(CAMERA_FOV_HASH, &reflected_fov, sizeof(reflected_fov)))      return false;
	if (!writeBlock(CAMERA_VERTICAL_HASH, &reflected_vertical, sizeof(reflected_vertical))) return false;
	if (!writeBlock(CAMERA_GAME_HASH, &reflected_isGameCam, sizeof(reflected_isGameCam)))return false;

	return out.good();
}

bool AG::Component::CameraComponent::Deserialize(std::istream& in) {
	while (in.good() && in.peek() != EOF) {
		uint32_t fieldHash = 0, fieldSize = 0;
		if (!IO::ReadUInt32(in, fieldHash)) break;
		if (!IO::ReadUInt32(in, fieldSize)) break;

		std::vector<char> buffer(fieldSize);
		if (fieldSize > 0 && !IO::ReadBytes(in, buffer.data(), fieldSize)) return false;

		auto eat = [&](auto& dst) {
			if (buffer.size() != sizeof(dst)) return false;
			std::memcpy(&dst, buffer.data(), sizeof(dst));
			return true;
			};

		switch (fieldHash) {
		case CAMERA_TYPE_HASH: { int   v = reflected_proj;       if (!eat(v)) return false; reflected_proj = v; break; }
		case CAMERA_NEAR_HASH: { float v = reflected_near;       if (!eat(v)) return false; reflected_near = v; break; }
		case CAMERA_FAR_HASH: { float v = reflected_far;        if (!eat(v)) return false; reflected_far = v; break; }
		case CAMERA_FOV_HASH: { float v = reflected_fov;        if (!eat(v)) return false; reflected_fov = v; break; }
		case CAMERA_VERTICAL_HASH: { float v = reflected_vertical;   if (!eat(v)) return false; reflected_vertical = v; break; }
		case CAMERA_GAME_HASH: { bool  v = reflected_isGameCam;  if (!eat(v)) return false; reflected_isGameCam = v; break; }
		default:
			// skip unknown future fields (buffer already consumed)
			break;
		}
	}

	SyncToCamera();
	return in.good() || in.eof();
}

REGISTER_REFLECTED_TYPE(AG::Component::CameraComponent)