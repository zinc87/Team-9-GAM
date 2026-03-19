/*!*****************************************************************************
\file ToolBar.h
\author Zulfami Ashrafi Bin Wakif
\date 20/1/2025 (MM/DD/YYYY)

\brief Tool Bar for the engine
*******************************************************************************/
#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace AGImGui {
		class ToolBarImgui : public AGImGui::IImguiWindow {
		public:

		public:
			ToolBarImgui() : AGImGui::IImguiWindow({ 200.f, 50.f }, "ToolBar") {}
			~ToolBarImgui() override;
			void Render() override;

		private:
		};
	}


	namespace System {

	}
}