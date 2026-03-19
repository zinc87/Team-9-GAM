#include "pch.h"
#include "PostProcessVolumeComponent.h"

void AG::Component::PostProcessVolumeComponent::Awake()
{
}

void AG::Component::PostProcessVolumeComponent::Start()
{
}

void AG::Component::PostProcessVolumeComponent::Update()
{
	if (effect_name.empty() || !enable_effect) return;

	auto trf_weak = GetObj().lock()->GetComponent<Component::TransformComponent>();
	auto trf_shared = trf_weak.lock();
	if (!trf_shared) return;

	// --- Update OBB ---
	glm::mat4 m2w = trf_shared->getM2W();
	m_ppvolume.center = glm::vec3(m2w[3]);
	m_ppvolume.rotation = glm::mat3(m2w);       // includes rotation + scale
	m_ppvolume.halfExtents = glm::vec3(0.5f);   // default, or assign volume siz


	auto gameCam = CameraManager::GetInstance().getGameCamera().lock();
	if (!gameCam) return;

	if (m_isGlobal)
	{
		PostProcesser::GetInstance().QueueEffect(effect_name);
	}
	else
	{
		auto camPos = gameCam->getCameraPosition();
		if (m_ppvolume.contains(camPos))
		{
			PostProcesser::GetInstance().QueueEffect(effect_name);
		}
	}

}

void AG::Component::PostProcessVolumeComponent::LateUpdate()
{
	auto gameCam = CameraManager::GetInstance().getGameCamera().lock();
	if (CameraManager::GetInstance().getCurrentCamera().lock() == gameCam || m_isGlobal) // render wireframe on scene
		return;

	auto trf_weak = GetObj().lock()->GetComponent<Component::TransformComponent>();
	auto trf_shared = trf_weak.lock();
	if (trf_shared)
	{
		DebugRenderer::GetInstance().DrawOBBWireframe(m_ppvolume, {1.f,1.f,0.f,1.f});
	}
}

void AG::Component::PostProcessVolumeComponent::Free()
{
}

void AG::Component::PostProcessVolumeComponent::Inspector()
{
	ImGui::Text("Effect:"); ImGui::SameLine();

	std::vector<const char*> c_effects = {""};
	auto s_effects = PostProcesser::GetInstance().GetEffectNames();
	for (auto& eff : s_effects)
	{
		c_effects.push_back(eff.c_str());
	}
	int eff_idx = 0;
	for (int i = 0; i < c_effects.size(); i++)
	{
		if (effect_name == c_effects[i])
		{
			eff_idx = i;
			break;
		}
	}

	if (ImGui::Combo("##effect_List", &eff_idx, c_effects.data(), (int)c_effects.size()))
	{
		effect_name = c_effects[eff_idx];
	}


	ImGui::Text("Is Global"); ImGui::SameLine();
	ImGui::Checkbox("##glob", &m_isGlobal);

	ImGui::Text("Enabled"); ImGui::SameLine();
	ImGui::Checkbox("##togg", &enable_effect);
}

void AG::Component::PostProcessVolumeComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
	ASSIGNFROM_FN_BODY(
		this->m_isGlobal = fromCmpPtr->m_isGlobal;
		this->m_ppvolume = fromCmpPtr->m_ppvolume;
		this->effect_name = fromCmpPtr->effect_name;
	)
}

REGISTER_REFLECTED_TYPE(AG::Component::PostProcessVolumeComponent)