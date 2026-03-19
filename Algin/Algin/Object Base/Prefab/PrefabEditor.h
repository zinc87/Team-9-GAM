#pragma once
#include "pch.h"

namespace AG
{

	class PrefabEditor : public Pattern::ISingleton<PrefabEditor>
	{
	public:
		void Enable() { m_enable = true; }
		bool IsEnabled() { if (!m_enable) Reset(); return m_enable; }
		void RenderGUI();

	private:

		void PickPrefab();
		void PrefabEditting();
		void Reset();
		bool m_enable = false;

		std::string prefab_name;
		Data::GUID pref_guid;
		std::string prefab_template_path;
		
		// for creation
		std::string c_prefab_name;
		std::string c_prefab_path;
		std::string c_prefab_template_path;
		bool creating_prefab = false;

		
		std::vector<std::shared_ptr<Component::IComponent>> m_comp_list;
	};
}

