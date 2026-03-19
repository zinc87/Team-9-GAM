#include "pch.h"
#include "PrefabEditor.h"

void AG::PrefabEditor::RenderGUI()
{


	ImGui::Begin("Prefab Editor", &m_enable);

	if (pref_guid.empty() && !creating_prefab)
		PickPrefab();
	else
		PrefabEditting();


	ImGui::End();
}

void AG::PrefabEditor::PickPrefab()
{
	ImVec2 content_space = ImGui::GetContentRegionAvail();

	std::vector<std::string> s_prefab_list{ "" };
	std::vector<const char*> prefab_list{};
	std::vector<Data::GUID> guid_list{ "" };
	for (const auto& temp : AG::System::PrefabManager::GetInstance().getIDToTemplate())
	{
		s_prefab_list.push_back(temp.second.getName());
		guid_list.push_back(temp.first);
	}

	for (auto& s_name : s_prefab_list)
	{
		prefab_list.push_back(s_name.c_str());
	}

	int prefab_index = 0;
	for (int i = 0; i < prefab_list.size(); i++)
	{
		if (prefab_name == prefab_list[i])
		{
			prefab_index = i;
			break;
		}
	}

	for (auto& names : prefab_list)
	{
		AG_CORE_INFO(names);
	}

	if (ImGui::Combo("##combo", &prefab_index, prefab_list.data(), (int)prefab_list.size()))
	{
		prefab_name = prefab_list[prefab_index];
		pref_guid = guid_list[prefab_index];
		m_comp_list.clear();
		auto& temp_jsonDoc = AG::System::PrefabManager::GetInstance().getIDToTemplate().at(pref_guid).getDoc();
		AG::System::SerializationSystem::GetInstance().deserializePrefabRootCmp(temp_jsonDoc,m_comp_list);
	}

	//if (ImGui::Button("Select Prefab", ImVec2(content_space.x, 25.f)))
	//{
	//	prefab_template_path = OpenFileDialog();
	//}

	if (!prefab_template_path.empty())
	{ // is it .prefab?
		std::filesystem::path fs_path(prefab_template_path);
		if (fs_path.extension().string() != ".prefab" || prefab_template_path.empty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.1f, 0.1f, 1.f), "Pick a valid Prefab Template file (.prefab)");
			prefab_template_path.clear();
		}
	}

	ImGui::SeparatorText("Create a new Prefab");

	ImGui::Text("Prefab Name:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::InputText("##new_prefab_name", &c_prefab_name);
	ImGui::Text("Prefab Directory:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
	ImGui::InputText("##new_prefab_dir", &c_prefab_path);
	ImGui::SameLine();
	if (ImGui::Button("##pick_dir", ImVec2(ImGui::GetContentRegionAvail().x, 25.f)))
	{
		c_prefab_path = OpenFolderDialog();
	}

	if (ImGui::Button("Create Prefab Template", ImVec2(content_space.x, 25.f)))
	{
		c_prefab_template_path = c_prefab_path + '\\' + c_prefab_name + ".prefab";
		creating_prefab = true;
	}
}

void AG::PrefabEditor::PrefabEditting()
{
	ImVec2 content_space = ImGui::GetContentRegionAvail();
	
	for (auto& comp : m_comp_list)
	{
		if (ImGui::CollapsingHeader(comp->GetTypeName().c_str()))
		{
			comp->Inspector();
		}
	}

	if (ImGui::Button("Save Prefab"))
	{

	}
}

void AG::PrefabEditor::Reset()
{
	prefab_name.clear();
	pref_guid.clear();
	prefab_template_path.clear();
	c_prefab_name.clear();
	c_prefab_path.clear();
	c_prefab_template_path.clear();
	creating_prefab = false;
}