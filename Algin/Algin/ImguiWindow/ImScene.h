/*****************************************************************//**
 * \file   ImScene.h
 * \brief  
 * 
 * \author Brandon
 * \date   September 2025
 *********************************************************************/
#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace AGImGui {
		class SceneView : public AGImGui::IImguiWindow {
		public:

		public:
			SceneView()  {}
			~SceneView() override;
			void Render() override;

			bool MouseInScene()
			{
				return	newMousePos.x >= imagePos.x + windowPos.x &&
					newMousePos.y >= imagePos.y + windowPos.y &&
					newMousePos.x <= imagePos.x + windowPos.x + imageSize.x &&
					newMousePos.y <= imagePos.y + windowPos.y + imageSize.y;
			}

			bool MouseInImageMapped() {
				return mappedMousePos.x <= imageSize.x &&
					mappedMousePos.x >= 0 &&
					mappedMousePos.y <= imageSize.y &&
					mappedMousePos.x >= 0;
			}

			void RenderTransformTool();

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
			ImVec2 mappedMousePos;
			ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
			ImVec2 TT_pos;
		};
	}
}