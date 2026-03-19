/*!*****************************************************************************
\file ObjHierarchy.h
\author Zulfami Ashrafi Bin Wakif
\date 20/1/2025 (MM/DD/YYYY)

\brief obj hierarchy for the engine
*******************************************************************************/
#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace AGImGui {
		class ObjHierarchy : public AGImGui::IImguiWindow {
		public:

		public:
			ObjHierarchy() : AGImGui::IImguiWindow({ 250.f, 50.f }, "Hierarchy") {}
			~ObjHierarchy() override;
			void Render() override;

		private:
			void DisplayObjectTree(const std::shared_ptr<System::IObject>& object, const std::string& filterText);

			std::shared_ptr<System::IObject> m_renamingObject = nullptr;
			char m_renameBuffer[128] = {};
			bool m_renameJustStarted = false;
		};
	}

}