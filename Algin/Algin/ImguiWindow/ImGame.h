/*****************************************************************//**
 * \file   ImGame.h
 * \brief  
 * 
 * \author Brandon
 * \date   September 2025
 *********************************************************************/
#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace AGImGui {
		class GameView : public AGImGui::IImguiWindow
		{
		public:

		public:
			GameView() {}
			~GameView() override;
			void Render() override;

			static std::pair<float, float> GetMouseNDC() { return mouseNDC; }


		private:
			std::shared_ptr<System::IObject> m_renamingObject = nullptr;
			char m_renameBuffer[128] = {};
			bool m_renameJustStarted = false;

			bool onMove = false;
			ImVec2 windowPos;
			ImVec2 imagePos;
			ImVec2 imageSize;

			ImVec2 oldMousePos;
			ImVec2 newMousePos;

			static std::pair<float, float> mouseNDC;
		};
	}
}