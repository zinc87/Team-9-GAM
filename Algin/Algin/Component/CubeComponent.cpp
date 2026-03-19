#include <algorithm>

#include "CubeComponent.h"
#include "pch.h"


using namespace AG;

void AG::Component::CubeComponent::Awake() {}

void AG::Component::CubeComponent::Start() {}

void AG::Component::CubeComponent::Update()
{
    auto trf_weak = GetObj().lock()->GetComponent<Component::TransformComponent>();
    auto trf_shared = trf_weak.lock();
    if (!trf_shared)
    {
        AG_CORE_WARN("Cube does not have Transform Component.");
        return;
    }

    m_cube->pos = trf_shared->GetTransformation().position;
    m_cube->rot = trf_shared->GetTransformation().rotation;
    m_cube->scale = trf_shared->GetTransformation().scale;

    r_position = m_cube->pos;
    r_rotation = m_cube->rot;
    r_scale = m_cube->scale;


    r_m_asset.params.baseColor = m_cube->params.baseColor;
    r_m_asset.params.mru_pad.x = m_cube->params.mru_pad.x;
    r_m_asset.params.mru_pad.y = m_cube->params.mru_pad.y;
}

void AG::Component::CubeComponent::LateUpdate()
{
    auto trf = GetObj().lock()->GetComponent<Component::TransformComponent>().lock();
    if (!trf) return;

    auto state = RenderPipeline::GetInstance().GetPipeline();

    if (state == RenderPipeline::OBJPICK) {
        Data::GUID objID = this->GetObjID();
        DebugRenderer::GetInstance().DrawCubeColor(trf->getM2W(), objID);
        return;
    }

    if (CAMERAMANAGER.getCurrentCamera().lock() == CAMERAMANAGER.getGameCamera().lock())
        return;

    if (state == RenderPipeline::LIGHT) {
        CubeRenderer::GetInstance().drawCube(trf->getM2W(), r_m_asset);
    }
}

void AG::Component::CubeComponent::Free() {}


void AG::Component::CubeComponent::Inspector()
{
    // Variables for material selection (kept your original logic)
    static int currentItem = 0;

    // 1. Material Combo (kept as is, but you can make this searchable too if you want)
    std::vector<const char*> materialNames;
    materialNames.push_back("Default Material");

    for (auto& material : MaterialSetup::GetInstance().materialAssets) {
        materialNames.push_back(material.name.c_str());
    }

    if (ImGui::Combo("Materials", &currentItem, materialNames.data(), (int)materialNames.size()))
    {
        if (currentItem == 0) {
            r_m_asset.params.baseColor = { 1.f, 1.f, 1.f, 1.f };
            r_m_asset.params.mru_pad.x = 0.f;
            r_m_asset.params.mru_pad.y = 0.f;
            r_m_asset.params.mru_pad.z = 1.f;
            r_m_asset.params.mru_pad.w = 1.f;
            r_m_asset.albedoHashID = 0;
            r_m_asset.normalMapHashID = 0;
            SyncToCube();
        }
        else {
            std::string matPath = MaterialSetup::GetInstance().materialPaths[currentItem - 1];
            MaterialAsset matAsset;
            if (MaterialSetup::GetInstance().parseMaterial(matPath, matAsset)) {
                r_m_asset.params.baseColor = matAsset.baseColor;
                r_m_asset.params.mru_pad.x = matAsset.metallic;
                r_m_asset.params.mru_pad.y = matAsset.roughness;
                r_m_asset.params.mru_pad.z = matAsset.UvScale;
                r_m_asset.params.mru_pad.w = matAsset.tint;
                r_m_asset.albedoHashID = matAsset.albedoHASHid;
                r_m_asset.normalMapHashID = matAsset.normalMapHashID;
                SyncToCube();
            }
            else {
                AG_CORE_ERROR("Failed to Parse Material File");
            }

        }
    }

    ImGui::Text("FOR DEBUGGING PURPOSES");

    // ---------------------------------------------------------
    // Prepare Texture List for Searchable Dropdowns
    // ---------------------------------------------------------
    auto& assetMgr = AssetManager::GetInstance();
    auto texAssets = assetMgr.GetAssets(ASSET_TYPE::DDS);
    std::vector<const char*> texNames;
    texNames.push_back("No Texture");
    for (auto& [name, asset] : texAssets)
        if (auto a = asset.lock()) texNames.push_back(a->asset_name.c_str());

    // ---------------------------------------------------------
    // Albedo Map (Searchable)
    // ---------------------------------------------------------
    std::string currentAlbedoName = "No Texture";
    // Find current name based on hash
    for (size_t i = 1; i < texNames.size(); ++i) {
        if (r_m_asset.albedoHashID == HASH2(texNames[i])) {
            currentAlbedoName = texNames[i];
            break;
        }
    }

    static ImGuiTextFilter albedoFilter;
    if (ImGui::BeginCombo("Albedo Map", currentAlbedoName.c_str()))
    {
        // Draw the search bar
        albedoFilter.Draw("##AlbedoFilter", -1.0f);

        // --- FIX: Force focus to the search bar when the popup first opens ---
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere(-1);
        // ----------------------------------------------------------------------

        ImGui::Separator();

        for (int i = 0; i < (int)texNames.size(); i++)
        {
            if (albedoFilter.PassFilter(texNames[i]))
            {
                bool isSelected = (currentAlbedoName == texNames[i]);
                if (ImGui::Selectable(texNames[i], isSelected))
                {
                    if (i == 0) r_m_asset.albedoHashID = 0;
                    else r_m_asset.albedoHashID = HASH2(texNames[i]);
                    SyncToCube();
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // ---------------------------------------------------------
    // Normal Map (Searchable)
    // ---------------------------------------------------------
    std::string currentNormalName = "No Texture";
    // Find current name based on hash
    for (size_t i = 1; i < texNames.size(); ++i) {
        if (r_m_asset.normalMapHashID == HASH2(texNames[i])) {
            currentNormalName = texNames[i];
            break;
        }
    }

    static ImGuiTextFilter normalFilter;
    if (ImGui::BeginCombo("Normal Map", currentNormalName.c_str()))
    {
        // Draw the search bar
        normalFilter.Draw("##NormalFilter", -1.0f);

        // --- FIX: Force focus to the search bar when the popup first opens ---
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere(-1);
        // ----------------------------------------------------------------------

        ImGui::Separator();

        for (int i = 0; i < (int)texNames.size(); i++)
        {
            if (normalFilter.PassFilter(texNames[i]))
            {
                bool isSelected = (currentNormalName == texNames[i]);
                if (ImGui::Selectable(texNames[i], isSelected))
                {
                    if (i == 0) r_m_asset.normalMapHashID = 0;
                    else r_m_asset.normalMapHashID = HASH2(texNames[i]);
                    SyncToCube();
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Sliders
    ImGui::SliderFloat3("Base Color", &r_m_asset.params.baseColor.x, 0.0f, 1.0f);
    ImGui::SliderFloat("Alpha", &r_m_asset.params.baseColor.a, 0.0f, 1.0f);
    ImGui::SliderFloat("Metallic", &r_m_asset.params.mru_pad.x, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness", &r_m_asset.params.mru_pad.y, 0.0f, 1.0f);
    ImGui::SliderFloat("UV Scale", &r_m_asset.params.mru_pad.z, 0.0f, 1.0f);
    ImGui::SliderFloat("Base Color Tint", &r_m_asset.params.mru_pad.w, 0.0f, 1.0f);
    SyncToCube();
}

void AG::Component::CubeComponent::AssignFrom([[maybe_unused]] const std::shared_ptr<IComponent>& fromCmp) {}

void AG::Component::CubeComponent::SyncToCube() {
    if (!m_cube) return;
    m_cube->params.baseColor = r_m_asset.params.baseColor;
    m_cube->params.mru_pad.x = r_m_asset.params.mru_pad.x;
    m_cube->params.mru_pad.y = r_m_asset.params.mru_pad.y;
    m_cube->params.mru_pad.z = r_m_asset.params.mru_pad.z;
    m_cube->albedoHashID = r_m_asset.albedoHashID;
    m_cube->normalMapHashID = r_m_asset.normalMapHashID;
}

REGISTER_REFLECTED_TYPE(AG::Component::CubeComponent)