#include "pch.h"
#include "SkyBoxComponent.h"

void AG::Component::SkyBoxComponent::Awake()
{

}

void AG::Component::SkyBoxComponent::Start()
{
    Self::GetCMItems();
    auto& hashes = CubemapHashesStorage();

    // find which index has this hash
    int foundIndex = 0; // default to "NO MESH"
    for (int i = 0; i < (int)hashes.size(); ++i) {
        if (hashes[i] == cubemap_hash) {
            foundIndex = i;
            break;
        }
    }

    cm_index = foundIndex;

    // Optional: update cubemapName for clarity/debug
    auto& names = CubemapNamesStorage();
    if (cm_index >= 0 && cm_index < (int)names.size()) {
        cubemapName = names[cm_index];
    }
}

void AG::Component::SkyBoxComponent::Update()
{

}

void AG::Component::SkyBoxComponent::LateUpdate()
{
    auto camera = CAMERAMANAGER.getCurrentCamera().lock();
    if (camera)
    {
		AG_CORE_INFO("Rendering Skybox with cubemap hash: {}", cubemap_hash);
        // look up cubemap by name -> hash -> asset
        SkyBoxRenderer::GetInstance().RenderSkyBox(
            camera->GetProjectionMatrix(),
            camera->GetInvViewMatrix(),
            cubemap_hash 
        );
    }
}

void AG::Component::SkyBoxComponent::Free()
{
}

void AG::Component::SkyBoxComponent::Inspector()
{

	std::vector<std::string> cubemaplist = {""};
	std::vector<const char*> cubemaplist_cstr = {};
	for (auto& cm : ASSET_MANAGER.GetAssets(AGCUBEMAP))
	{
		cubemaplist.push_back(cm.second.lock()->asset_name);
	}

    for (auto& name : cubemaplist)
    {
        cubemaplist_cstr.push_back(name.c_str());
	}

	cm_index = 0;
	if (!cubemapName.empty() || cubemapName != "No Map")
	{
		for (int i = 0; i < cubemaplist.size(); i++)
		{
            if (cubemap_hash == HASH(cubemaplist[i]))
            {
                cm_index = i;
                break;
            }
		}
	}
	if (ImGui::Combo("Cube Map", &cm_index, cubemaplist_cstr.data(), (int)cubemaplist_cstr.size()))
	{
		cubemap_hash = HASH(cubemaplist[cm_index]);
	}

    /*
	auto trf_weak = GetObj().lock()->GetComponent<Component::TransformComponent>();
	auto trf_shared = trf_weak.lock();
	if (trf_shared)
	{
		ImGui::Text("Exposure"); ImGui::SameLine();
		ImGui::SliderFloat("##exp", &trf_shared->GetTransformation().rotation.x, -89.f, 90.f);
	}*/

	// tint color
// exposure
// rotation

    // Cube map combo (custom UI)
    //std::vector<std::string> cubemaplist = { "No Map" };
    //for (auto& cm : ASSET_MANAGER.GetAssets(AGCUBEMAP))
    //{
    //    cubemaplist.push_back(cm.first);
    //}

    //int cm_index = 0;
    //if (!cubemapName.empty())
    //{
    //    for (int i = 0; i < (int)cubemaplist.size(); i++)
    //    {
    //        if (cubemapName == cubemaplist[i])
    //            cm_index = i;
    //    }
    //}
    //if (ImGuiCustom::SearchableCombo("Cube Map", &cm_index, cubemaplist))
    //{	
    //    cubemapName = cubemaplist[cm_index];
    //}

    // Render the rest of the reflected fields (exposure, tint, rotation)
}

void AG::Component::SkyBoxComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
    ASSIGNFROM_FN_BODY(
        this->cm_index = fromCmpPtr->cm_index;
        this->m_exposure = fromCmpPtr->m_exposure;
        this->m_tint = fromCmpPtr->m_tint;
        this->m_rotation = fromCmpPtr->m_rotation;
    )
}

REGISTER_REFLECTED_TYPE(AG::Component::SkyBoxComponent)
