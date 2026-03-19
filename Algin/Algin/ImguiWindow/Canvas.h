#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace AGImGui {
		class Canvas : public AGImGui::IImguiWindow {
		public:

		public:
			Canvas() {}
			~Canvas() override {}
			void Render() override;
			void RenderTransformTool();
		private:
			bool onMove = false;
			ImVec2 windowPos;
			ImVec2 imagePos;
			ImVec2 imageSize;

			ImVec2 oldMousePos;
			ImVec2 newMousePos;

			ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
			ImVec2 TT_pos;
		};
	}
}