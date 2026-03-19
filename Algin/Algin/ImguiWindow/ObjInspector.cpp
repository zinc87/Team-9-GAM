/*!*****************************************************************************
\file ObjInspector.cpp
\author Zulfami Ashrafi Bin Wakif
\date 20/1/2025 (MM/DD/YYYY)

\brief Imgui Inspector for the engine
*******************************************************************************/
#pragma once
#include "pch.h"

namespace AG {
	namespace AGImGui {
        ObjInspector::~ObjInspector() {

        }


        void ObjInspector::Render() {

            if (m_editingMaterial)
            {
                MaterialInspector();
            }
            else if (m_editingMesh)
            {
                MeshInspector();
			}
            else if (m_editingSkinned)
            {
                MeshInspectorSkinned();
            }
            else
            {
                ImGui::SetNextWindowSize(ImVec2(m_size.first, m_size.second), ImGuiCond_FirstUseEver);

                if (!ImGui::Begin(m_name.c_str())) {
                    ImGui::End();
                    return;
                }

                // Selected object
                G_selectedObj = IMGUISYSTEM.GetSelectedObj().lock();
                if (!G_selectedObj) {
                    ImGui::TextDisabled("No object selected.");
                    ImGui::End();
                    return;
                }

                // Header
                ImGui::Checkbox("##Enable", &G_selectedObj->IsEnable());


                ImGui::SameLine();
                ImGui::InputText("##ObjName", &G_selectedObj->Name()); // TODO: command pattern
                ImGui::Separator();


                // Load all component inspector
                for (auto& weak : G_selectedObj.get()->GetComponentsType())
                {
                    if (auto comp = weak.lock())
                    {
                        // Unique ID scope for this component's widgets + popup
                        ImGui::PushID(comp->GetID().c_str());

                        ImGui::Checkbox("##enabled1", &comp->IsEnable());
                        ImGui::SameLine();
                        std::string title = Data::ComponentTypeToString(comp->GetType());
                        const char* label = title.c_str();
                        if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            // Right-click popup bound to the header item
                            if (ImGui::BeginPopupContextItem("CompCtxMenu", ImGuiPopupFlags_MouseButtonRight))
                            {
                                if (ImGui::MenuItem("Remove Component"))
                                {
                                    // queue removal (type, id) after UI pass
                                    AG::System::ComponentManager::GetInstance().RemoveComponent(comp->GetID());
                                }
                                ImGui::EndPopup();
                            }

                            if (ImGui::IsItemHovered())
                            {
                                ImGui::BeginTooltip();

                                std::string att_obj_id = static_cast<std::string>(comp->GetObjID());
                                std::string curr_comp_id = static_cast<std::string>(comp->GetID());

                                ImGui::Text("Component ID: \t\t\t%s", curr_comp_id.c_str());
                                ImGui::Text("Attached Object ID: \t%s", att_obj_id.c_str());

                                ImGui::EndTooltip();
                            }

                            ImGui::BeginDisabled(!comp->IsEnable() || !G_selectedObj->IsEnable());

                            comp->Inspector();
                            ImGui::Separator();

                            ImGui::EndDisabled();
                        }
                        else
                        {
                            // Even when closed, allow right-click on the header to open the menu
                            if (ImGui::BeginPopupContextItem("CompCtxMenu", ImGuiPopupFlags_MouseButtonRight))
                            {
                                if (ImGui::MenuItem("Remove Component"))
                                {
                                    AG::System::ComponentManager::GetInstance().RemoveComponent(comp->GetID());
                                }
                                ImGui::EndPopup();
                            }
                        }
                        ImGui::PopID();
                    }
                }

                // --- Add Component button centered ---
                float windowWidth = ImGui::GetContentRegionAvail().x;
                const char* btnTxt = "Add Component";
                const float buttonWidthPercentage = 0.8f;

                float buttonWidth = windowWidth * buttonWidthPercentage;
                ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

                static ImVec2 g_AddCompPopupPos{ 0,0 };   // anchor position (left,bottom of button)
                if (ImGui::Button(btnTxt, ImVec2(buttonWidth, 0))) {
                    // Capture the button rect to anchor the popup directly below it
                    ImVec2 min = ImGui::GetItemRectMin();
                    ImVec2 max = ImGui::GetItemRectMax();
                    g_AddCompPopupPos = ImVec2(min.x, max.y);  // left edge, just under the button
                    ImGui::OpenPopup("AddComponentPopup");
                }

                // Always push the next window pos so the popup stays pinned and can't be moved
                ImGui::SetNextWindowPos(g_AddCompPopupPos, ImGuiCond_Always);

                // Optional: force width to match button (looks like a dropdown)
                float popupWidth = buttonWidth; // or use ImGui::GetContentRegionAvail().x for full width
                ImGui::SetNextWindowSize(ImVec2(popupWidth, 0), ImGuiCond_Appearing); // 0 height = auto

                // Make it immovable (and tidy like a dropdown)
                ImGuiWindowFlags popupFlags =
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_AlwaysAutoResize; // auto height as content grows

                if (ImGui::BeginPopup("AddComponentPopup", popupFlags))
                {
                    // --- Search bar ---
                    static char search[128] = { 0 };
                    ImGui::SetNextItemWidth(-1.0f);
                    ImGui::InputTextWithHint("##addCompSearch", "Search components...", search, IM_ARRAYSIZE(search));

                    auto icontains = [](std::string_view hay, std::string_view needle) {
                        if (needle.empty()) return true;
                        auto lower = [](unsigned char c) { return char(std::tolower(c)); };
                        std::string h(hay); std::transform(h.begin(), h.end(), h.begin(), lower);
                        std::string n(needle); std::transform(n.begin(), n.end(), n.begin(), lower);
                        return h.find(n) != std::string::npos;
                        };
                    const std::string query = search;

                    // Build set of already-attached types for this object
                    std::unordered_set<AG::System::ComponentManager::CType> existing;
                    {
                        const auto& objMap = COMPONENTMANAGER.GetComponentsObj();
                        auto it = objMap.find(G_selectedObj->GetID());
                        if (it != objMap.end()) {
                            for (auto& wptr : it->second) if (auto sp = wptr.lock()) existing.insert(sp->GetType());
                        }
                    }

                    // List all registered types
                    const auto& factories = COMPONENTMANAGER.GetFactories();

                    ImGui::BeginChild("##AddCompList", ImVec2(0, 260), true);

                    struct Row { AG::System::ComponentManager::CType type; std::string label; };
                    std::vector<Row> rows; rows.reserve(factories.size());

                    for (const auto& [type, ctor] : factories) {
                        if (!ctor) continue;
                        if (existing.count(type)) continue;

                        std::string label = AG::Data::ComponentTypeToString(type);
                        if (!icontains(label, query)) continue;

                        rows.push_back({ type, std::move(label) });
                    }

                    if (rows.empty()) {
                        ImGui::TextDisabled("No components found.");
                    }
                    else {
                        ImGuiListClipper clipper;
                        clipper.Begin((int)rows.size());
                        while (clipper.Step()) {
                            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                                const auto& row = rows[i];
                                if (ImGui::Selectable(row.label.c_str())) {
                                    COMPONENTMANAGER.CreateComponent(G_selectedObj.get(), row.type);
                                    ImGui::CloseCurrentPopup();
                                    break;
                                }
                            }
                        }
                    }

                    ImGui::EndChild();

                    // Footer
                    if (ImGui::Button("Clear")) search[0] = '\0';
                    ImGui::SameLine();
                    if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();

                    ImGui::EndPopup();
                }

                //float m_gamma = PostProcesser::GetInstance().GetGammaScaling();
                //ImGui::DragFloat("Gamma", &m_gamma, 0.01f, 0.1f, 10.0f, "%.2f");
                //PostProcesser::GetInstance().SetGammaScaling(m_gamma);

                //float m_exposure = PostProcesser::GetInstance().GetExposure(); 
                //ImGui::DragFloat("Exposure", &m_exposure, 0.01f, 0.1f, 10.0f, "%.2f");
                //PostProcesser::GetInstance().SetExposure(m_exposure);


                ImGui::End();
            }
        }

        void ObjInspector::MaterialInspector()
        {
            ImGui::Begin("Inspector");

            // Static filter to persist search text while the menu is open/interacted with
            static ImGuiTextFilter filter;

            if (!m_editingMaterial)
            {
                ImGui::End();
                return;
            }

            auto mat_shr = m_editingMaterial;
            bool is_instance = mat_shr->reference_mat != 0;

            std::vector<std::string> s_tex = { "" };
            for (auto& tex : AssetManager::GetInstance().GetAssets(ASSET_TYPE::DDS))
            {
				auto tex_sp = tex.second.lock();
                if (tex_sp)
                {
                    s_tex.push_back(tex_sp->asset_name);
                }
            }
            std::vector<const char*> c_tex;
            for (auto& str : s_tex)
            {
                c_tex.push_back(str.c_str());
            }


            float width = ImGui::GetContentRegionAvail().x;

            if (ImGui::Button("Close"))
            {
                m_editingMaterial.reset();
                ImGui::End();
                return;
            }

            ImGui::Text("Name: "); ImGui::SameLine();
            ImGui::SetCursorPosX(width * 0.25f);
            ImGui::InputText("##mat_inp_name", &temp_mat_name);

            ImGui::Text("Path : "); ImGui::SameLine();
            if (ImGui::Button("Select"))
            {
                current_path = OpenFileDialog();
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(width * 0.25f);
			ImGui::InputText("##mat_inp_path", &current_path, ImGuiInputTextFlags_ReadOnly);

			auto shaders_string = ShaderManager::GetInstance().getShaderNames();
            std::vector<const char*> c_shaders = {""};
			int shader_idx = 0;

            for (auto& str : shaders_string)
            {
                c_shaders.push_back(str.c_str());
            }

            for (int i = 0; i < shaders_string.size(); i++)
            {
                if (mat_shr->shader_name == c_shaders[i])
                {
                    shader_idx = i;
                    break;
				}
            }


			ImGui::Text("Shader: "); ImGui::SameLine();
            ImGui::SetCursorPosX(width * 0.25f);
            if (ImGui::Combo("##mat_shader", &shader_idx, c_shaders.data(), (int)c_shaders.size()))
            {
				mat_shr->shader_name = c_shaders[shader_idx];
            }

            if (is_instance)
            {
                auto ref_mat = AssetManager::GetInstance().GetAsset(mat_shr->reference_mat);
                if (auto ref_mat_sp = std::dynamic_pointer_cast<MaterialTemplate>(ref_mat.lock()))
                {
                    ImGui::Text("Ref.Material: "); ImGui::SameLine();
                    ImGui::SetCursorPosX(width * 0.25f);
                    ImGui::Text("%s", ref_mat_sp->asset_name.c_str());
                }
            }

            //ALBEDO BOX

            ImGui::SetCursorPosX((width - ImGui::CalcTextSize("Albedo").x) * 0.5f);
            ImGui::Text("Albedo");
            if (is_instance)
            {
                ImGui::SameLine();
				ImGui::Checkbox("Override Albedo", &mat_shr->override_albedo);
            }
            ImGui::BeginDisabled(is_instance && !mat_shr->override_albedo);
            ImGui::BeginChild("##Albedo", ImVec2(width, 80.f), 1);
            ImTextureID albedo_id = 0;
            std::string albedo_name;
            auto albedo_wk = AssetManager::GetInstance().GetAsset(mat_shr->GetAlbedoHash());
            if (albedo_wk.lock())
            {
                auto albedo_shr = std::dynamic_pointer_cast<TextureAsset>(albedo_wk.lock());
                if (albedo_shr)
                {
                    albedo_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(albedo_shr->textureID));
                    albedo_name = albedo_shr->asset_name;
                }
            }
            ImGui::Image(albedo_id, ImVec2(63.f, 63.f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::SameLine();
            int albedo_idx = 0;
            for (int i = 0; i < s_tex.size(); i++)
            {
                if (mat_shr->GetAlbedoHash() == HASH(s_tex[i]))
                {
                    albedo_idx = i;
                    break;
                }
            }

            // Keep these persistent somewhere (static, or as members)
            static ImGuiTextFilter albedoFilter;
            static bool focusFilterOnOpen = false;

            // Current preview string
            const char* preview = (albedo_idx >= 0 && albedo_idx < (int)s_tex.size())
                ? s_tex[albedo_idx].c_str()
                : "<invalid>";

            if (ImGui::BeginCombo("##pickAlbedo", preview))
            {   
                // Detect "just opened" and focus the search box once
                if (!focusFilterOnOpen) {
                    focusFilterOnOpen = true;
                    ImGui::SetKeyboardFocusHere(); // focuses next item (the filter input)
                }

                albedoFilter.Draw("##AlbedoSearch", -1.0f);
                ImGui::Separator();
                ImGui::BeginChild("##AlbedoList", ImVec2(0, 200), false);

                for (int n = 0; n < (int)s_tex.size(); ++n) {

                    if (!albedoFilter.PassFilter(s_tex[n].c_str()))
                        continue;

                    bool isSelected = (albedo_idx == n);
                    if (ImGui::Selectable(s_tex[n].c_str(), isSelected))
                        albedo_idx = n;

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();

                }



                ImGui::EndChild();
                ImGui::EndCombo();
            }
            else {
                // Closed -> reset the "focus once" flag
                focusFilterOnOpen = false;
            }
            if (albedo_idx == 0) {
                mat_shr->GetAlbedoHash() = 0;
            }
            else {
                mat_shr->GetAlbedoHash() = HASH(s_tex[albedo_idx]);
            }

            ImGui::EndChild();

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("IMGUI_CONTENT"))
                {
					//AG_CORE_INFO("[DragDrop] Received payload: {0} {1}", std::string((const char*)payload->Data, payload->DataSize), HASH(std::string((const char*)payload->Data)));
                    mat_shr->GetAlbedoHash() = HASH(std::string((const char*)payload->Data, payload->DataSize));
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::EndDisabled();


            //NORMAL MAP BOX

            ImGui::SetCursorPosX((width - ImGui::CalcTextSize("Normal Map").x) * 0.5f);
            ImGui::Text("Normal Map");
            if (is_instance)
            {
                ImGui::SameLine();
                ImGui::Checkbox("Override Normal", &mat_shr->override_normal);
            }
            ImGui::BeginDisabled(is_instance && !mat_shr->override_normal);
            ImGui::BeginChild("##NormalMap", ImVec2(width, 80.f), 1);
            ImTextureID normal_id = 0;
            std::string normal_name;
            auto normal_wk = AssetManager::GetInstance().GetAsset(mat_shr->GetNormalHash());
            if (normal_wk.lock())
            {
                auto normal_shr = std::dynamic_pointer_cast<TextureAsset>(normal_wk.lock());
                if (normal_shr)
                {
                    normal_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(normal_shr->textureID));
                    normal_name = normal_shr->asset_name;
                }
            }
            ImGui::Image(normal_id, ImVec2(63.f, 63.f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::SameLine();
            int normal_idx = 0;
            for (int i = 0; i < s_tex.size(); i++)
            {
                if (mat_shr->GetNormalHash() == HASH(s_tex[i]))
                {
                    normal_idx = i;
                    break;
                }
            }

            if (ImGui::Combo("##pickNormal", &normal_idx, c_tex.data(), (int)c_tex.size()))
            {

                if (normal_idx == 0)
                {
                    mat_shr->GetNormalHash() = 0;
                }
                else
                {
                    mat_shr->GetNormalHash() = HASH(s_tex[normal_idx]);
                }

            }

            ImGui::EndChild();
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("IMGUI_CONTENT"))
                {
                    mat_shr->GetNormalHash() = HASH(std::string((const char*)payload->Data));
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::EndDisabled();




            //BASE COLOR
            ImGui::SetCursorPosX((width - ImGui::CalcTextSize("Base Color").x) * 0.5f);
            ImGui::Text("Base Color");
            if (is_instance)
            {
                ImGui::SameLine();
                ImGui::Checkbox("Override Base Color", &mat_shr->override_baseColor);
            }
            ImGui::BeginDisabled(is_instance && !mat_shr->override_baseColor);
            ImGui::BeginChild("##BaseColor", ImVec2(width, 380.f), ImGuiChildFlags_Borders);
            ImGui::ColorPicker4("##color", &mat_shr->GetBaseColor().r);
            ImGui::EndChild();
            ImGui::EndDisabled();


            // Metallic
            ImGui::Text("Metallic ");
            if (is_instance)
            {
                ImGui::SameLine();
                ImGui::Checkbox("Override Metallic", &mat_shr->override_metallic);
            }
            ImGui::BeginDisabled(is_instance && !mat_shr->override_metallic);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::SliderFloat("##metal", &mat_shr->GetMetallic(), 0.f, 1.f);
            ImGui::EndDisabled();


            ImGui::Text("Roughness ");
            if (is_instance)
            {
                ImGui::SameLine();
                ImGui::Checkbox("Override Roughness", &mat_shr->override_roughness);
            }
            ImGui::BeginDisabled(is_instance && !mat_shr->override_roughness);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::SliderFloat("##rough", &mat_shr->GetRoughness(), 0.f, 1.f);
            ImGui::EndDisabled();


            ImGui::Text("UV Scale ");
            if (is_instance)
            {
                ImGui::SameLine();
                ImGui::Checkbox("Override UV Scale", &mat_shr->override_uv_scale);
            }
            ImGui::BeginDisabled(is_instance && !mat_shr->override_uv_scale);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::SliderFloat("##uv", &mat_shr->GetUV_scale(), 0.f, 1.f);
            ImGui::EndDisabled();

            ImGui::Text("Base Color Tint ");
            if (is_instance)
            {
                ImGui::SameLine();
                ImGui::Checkbox("Override Tint", &mat_shr->override_tint);
            }
            ImGui::BeginDisabled(is_instance && !mat_shr->override_tint);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::SliderFloat("##tint", &mat_shr->GetTint(), 0.f, 1.f);
            ImGui::EndDisabled();

            if (ImGui::Button("Save Material"))
            {
                std::string fn = temp_mat_name + ".agmat";
                std::string fp = current_path + "\\";
                mat_shr->asset_name = fn;
                mat_shr->material_name = temp_mat_name;
                mat_shr->SaveToJson(fp + fn);
                mat_shr->LoadFromJson(fp + fn);
				AssetManager::GetInstance().RegisterAsset(AGMATERIAL, HASH(fn), mat_shr);
            }

            ImGui::End();
        }
        void ObjInspector::MeshInspector()
        {
			ImGui::Begin("Inspector");

            if (!m_editingMesh)
            {
                ImGui::End();
                return;
            }

            if (ImGui::Button("Close"))
            {
                m_editingMaterial.reset();
                ImGui::End();
                return;
            }

			auto mesh_shr = m_editingMesh;
			ImGui::Text("Name: %s", mesh_shr->asset_name.c_str());

            if (ImGui::CollapsingHeader("Submeshes"))
            {
				auto& submeshes = mesh_shr->submeshes;

                for (int i = 0 ; i < submeshes.size(); i++)
                {
					ImGui::Text("%i. ", i); ImGui::SameLine();
					ImGui::InputText(("##submesh_name" + std::to_string(i)).c_str(), &submeshes[i].name);
				}

            }

            if (ImGui::Button("Save Mesh Settings To File"))
            {
                m_editingMesh->WriteToBinary(m_editingMesh->ori_path);
            }


            ImGui::End();
        }
        void ObjInspector::MeshInspectorSkinned()
        {
            ImGui::Begin("Inspector");

            if (!m_editingSkinned)
            {
                ImGui::End();
                return;
            }

            if (ImGui::Button("Close"))
            {
                m_editingSkinned.reset();
                ImGui::End();
                return;
            }

            auto mesh_shr = m_editingSkinned;
            ImGui::Text("Name: %s", mesh_shr->asset_name.c_str());

            if (ImGui::CollapsingHeader("Submeshes"))
            {
                auto& submeshes = mesh_shr->submeshes;

                for (int i = 0; i < submeshes.size(); i++)
                {
                    ImGui::Text("%i. ", i); ImGui::SameLine();
                    ImGui::InputText(("##submesh_name" + std::to_string(i)).c_str(), &submeshes[i].name);
                }

            }

            if (ImGui::CollapsingHeader("Animations"))
            {
                auto& ani_clip = mesh_shr->animations;
                
                for (int i = 0; i < ani_clip.size(); i++)
                {
                    ImGui::Text("%i. ", i); ImGui::SameLine();
                    ImGui::InputText(("##animation_name" + std::to_string(i)).c_str(), &ani_clip[i].name);
                }

            }


            if (ImGui::Button("Save Mesh Settings To File"))
            {
                m_editingSkinned->WriteToBinary(m_editingSkinned->ori_path);
				AG_CORE_INFO("Saved Skinned Mesh Settings to file: {}", m_editingSkinned->ori_path);
            }


            ImGui::End();
        }
	}
}