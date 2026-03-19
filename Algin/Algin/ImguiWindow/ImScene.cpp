#include "pch.h"
#include "ImScene.h"

AG::AGImGui::SceneView::~SceneView()
{
}

void AG::AGImGui::SceneView::Render()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
	bool inScene = ImGui::Begin("Scene", nullptr, windowFlags);

	// Add a red box when the game is running
	if (SCENESTATEMANAGER.IsSceneRunning())
	{
		const ImVec2 winPos = ImGui::GetWindowPos();   // screen-space
		const ImVec2 winSize = ImGui::GetWindowSize();
		const float  rounding = ImGui::GetStyle().WindowRounding;
		const float  thickness = 2.0f * ImGui::GetIO().FontGlobalScale;
		const float	 pixelOffset = 0.5f;
		const float  tabOffset = 21.0f;

		// Nudge by 0.5px so the line sits crisply on pixel boundaries
		const ImVec2 pMin = ImVec2(winPos.x + pixelOffset, winPos.y + tabOffset);
		const ImVec2 pMax = ImVec2(winPos.x + winSize.x - pixelOffset, winPos.y + winSize.y - pixelOffset);

		ImGui::GetWindowDrawList()->AddRect(
			pMin, pMax,
			IM_COL32(255, 0, 0, 255), // red
			rounding,
			0,
			thickness
		);

	}


	windowPos = ImGui::GetWindowPos();
	//focused = ImGui::IsWindowFocused();

	ImVec2 availSize = ImGui::GetContentRegionAvail();
	ImVec2 cursorPos = ImGui::GetCursorPos();

	float image_aspectR = static_cast<float>(AGWINDOW.getAspectRatio().first) /
		static_cast<float>(AGWINDOW.getAspectRatio().second);
	float window_aspectR = availSize.x / availSize.y;

	if (window_aspectR < image_aspectR)
	{
		imageSize.x = availSize.x;
		imageSize.y = imageSize.x / image_aspectR;
	}
	else
	{
		imageSize.y = availSize.y;
		imageSize.x = imageSize.y * image_aspectR;
	}

	imagePos.x = cursorPos.x + (availSize.x - imageSize.x) * 0.5f;
	imagePos.y = cursorPos.y + (availSize.y - imageSize.y) * 0.5f;

	ImGui::SetCursorPos(imagePos);
	ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(AGWINDOW.getFramebuffer("Scene"))), imageSize, ImVec2{ 0,1 }, ImVec2{ 1,0 });

	ImVec2 imgMin = ImGui::GetItemRectMin();
	ImVec2 imgMax = ImGui::GetItemRectMax();

	/* ---- ImGuizmo ---- */
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(windowPos.x + imagePos.x, windowPos.y + imagePos.y, imageSize.x, imageSize.y);
	/* ---- ImGuizmo ---- */

	auto selected = IMGUISYSTEM.GetSelectedObj();
	if (selected.lock())
	{
		auto trf_weak = selected.lock()->GetComponent<Component::TransformComponent>();
		auto selected_shared = trf_weak.lock();
		if (selected_shared)
		{
			auto m2w = selected_shared->getM2W();
			ImGuizmo::Manipulate(
				glm::value_ptr(CAMERAMANAGER.getSceneCamera()->GetViewMatrix()),
				glm::value_ptr(CAMERAMANAGER.getSceneCamera()->GetProjectionMatrix()),
				gizmoOperation,
				ImGuizmo::WORLD,
				glm::value_ptr(m2w));

			if (ImGuizmo::IsUsing())
			{

				float old_pos[3], old_rotDeg[3], old_scl[3];
				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(selected_shared->getM2W()), old_pos, old_rotDeg, old_scl);
				// Decompose
				float pos[3], rotDeg[3], scl[3];
				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(m2w), pos, rotDeg, scl);

				// Normalize angles to [-180, 180]
				for (int i = 0; i < 3; ++i) {
					while (rotDeg[i] > 180.0f) rotDeg[i] -= 360.0f;
					while (rotDeg[i] < -180.0f) rotDeg[i] += 360.0f;
				}

				// get difference
				
				glm::vec3 deltaPos = glm::vec3(pos[0] - old_pos[0], pos[1] - old_pos[1], pos[2] - old_pos[2]);
				glm::vec3 deltaRot = glm::vec3(rotDeg[0] - old_rotDeg[0], rotDeg[1] - old_rotDeg[1], rotDeg[2] - old_rotDeg[2]);
				glm::vec3 deltaScl = glm::vec3(scl[0] - old_scl[0], scl[1] - old_scl[1], scl[2] - old_scl[2]);

				
				// Assign

				selected_shared->GetTransformation().position += deltaPos;
				selected_shared->GetTransformation().rotation += deltaRot;
				selected_shared->GetTransformation().scale += deltaScl;

			}
		}

	}

	RenderTransformTool();

	ImGui::End();


	CAMERAMANAGER.getSceneCamera()->UpdateView();
	CAMERAMANAGER.getSceneCamera()->UpdateProjection();

	// * scene control

	float scroll = ImGui::GetIO().MouseWheel;
	if (MouseInScene() && scroll != 0.0f)
	{
		CAMERAMANAGER.getSceneCamera()->getCameraPosition() += CAMERAMANAGER.getSceneCamera()->getCameraDirection() * (scroll * 0.2f);
	}




	newMousePos = ImGui::GetIO().MousePos;
	mappedMousePos.x = std::floor(newMousePos.x - imgMin.x);
	mappedMousePos.y = std::floor(imgMax.y - newMousePos.y);



	ScanSetup::GetInstance().scanMousePos = mappedMousePos;
	ScanSetup::GetInstance().viewDims = imageSize;
	if (!ImGuizmo::IsUsing()  && MouseInImageMapped() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && inScene) {
		objectPicking::GetInstance().renderPickPass(mappedMousePos, imageSize);
	}



	if ((MouseInScene() && (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseDown(ImGuiMouseButton_Middle))) || onMove)
	{
		onMove = true;
	}

	if ((!ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGui::IsMouseDown(ImGuiMouseButton_Middle)) || !onMove)
	{
		onMove = false;
		oldMousePos = newMousePos;
		return;
	}

	float speed = 0.05f * (ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 2.f : 1.f);

	ImVec2 offset = newMousePos - oldMousePos;
	offset = offset / imageSize;
	offset *= 100.f;
	if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		CAMERAMANAGER.getSceneCamera()->getCameraRotation()[1] += offset.x;
		CAMERAMANAGER.getSceneCamera()->getCameraRotation()[0] -= offset.y;
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
	{
		CAMERAMANAGER.getSceneCamera()->getCameraPosition() += CAMERAMANAGER.getSceneCamera()->getCameraUp() * offset.y * speed;
		CAMERAMANAGER.getSceneCamera()->getCameraPosition() -= CAMERAMANAGER.getSceneCamera()->getCameraRight() * offset.x * speed;
	}



	if (ImGui::IsKeyDown(ImGuiKey_W) && ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		CAMERAMANAGER.getSceneCamera()->getCameraPosition() += CAMERAMANAGER.getSceneCamera()->getCameraDirection() * speed;
	}

	if (ImGui::IsKeyDown(ImGuiKey_S) && ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		CAMERAMANAGER.getSceneCamera()->getCameraPosition() -= CAMERAMANAGER.getSceneCamera()->getCameraDirection() * speed;
	}

	if (ImGui::IsKeyDown(ImGuiKey_A) && ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		CAMERAMANAGER.getSceneCamera()->getCameraPosition() -= glm::normalize(glm::cross(CAMERAMANAGER.getSceneCamera()->getCameraDirection(), glm::vec3(0.0f, 1.0f, 0.0f))) * speed;
	}

	if (ImGui::IsKeyDown(ImGuiKey_D) && ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		CAMERAMANAGER.getSceneCamera()->getCameraPosition() += glm::normalize(glm::cross(CAMERAMANAGER.getSceneCamera()->getCameraDirection(), glm::vec3(0.0f, 1.0f, 0.0f))) * speed;
	}

	if (ImGui::IsKeyDown(ImGuiKey_Space) && ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		CAMERAMANAGER.getSceneCamera()->getCameraPosition() += glm::vec3(0.f, 1.f, 0.f) * speed;
	}

	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		CAMERAMANAGER.getSceneCamera()->getCameraPosition() -= glm::vec3(0.f, 1.f, 0.f) * speed;
	}


	CAMERAMANAGER.getSceneCamera()->UpdateView();
	CAMERAMANAGER.getSceneCamera()->UpdateProjection();
	oldMousePos = newMousePos;
}

void AG::AGImGui::SceneView::RenderTransformTool()
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 window_pos = ImGui::GetWindowPos();
	ImVec2 content_min = ImGui::GetWindowContentRegionMin();
	ImVec2 content_max = ImGui::GetWindowContentRegionMax();

	ImVec2 scene_min = window_pos + content_min;
	ImVec2 scene_max = window_pos + content_max;

	ImVec2 rect_size(120, 40);

	static bool initialized = false;
	static ImVec2 TT_offset(0, 0); // relative to scene_min

	if (!initialized) {
		TT_offset = ImVec2(0, 0); // start pinned to top-left
		initialized = true;
	}

	// Convert offset to absolute each frame
	ImVec2 widgetPos = scene_min + TT_offset;

	// Clamp inside bounds
	if (widgetPos.x < scene_min.x) widgetPos.x = scene_min.x;
	if (widgetPos.y < scene_min.y) widgetPos.y = scene_min.y;
	if (widgetPos.x + rect_size.x > scene_max.x) widgetPos.x = scene_max.x - rect_size.x;
	if (widgetPos.y + rect_size.y > scene_max.y) widgetPos.y = scene_max.y - rect_size.y;

	// Mouse input
	ImVec2 mouse = ImGui::GetIO().MousePos;
	bool hovered =
		mouse.x >= widgetPos.x && mouse.x <= widgetPos.x + rect_size.x &&
		mouse.y >= widgetPos.y && mouse.y <= widgetPos.y + rect_size.y;

	static bool dragging = false;
	static ImVec2 dragOffset(0, 0);

	if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		dragging = true;
		dragOffset = mouse - widgetPos;
	}

	if (dragging) {
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			widgetPos = mouse - dragOffset;

			// Clamp while dragging
			if (widgetPos.x < scene_min.x) widgetPos.x = scene_min.x;
			if (widgetPos.y < scene_min.y) widgetPos.y = scene_min.y;
			if (widgetPos.x + rect_size.x > scene_max.x) widgetPos.x = scene_max.x - rect_size.x;
			if (widgetPos.y + rect_size.y > scene_max.y) widgetPos.y = scene_max.y - rect_size.y;
		}
		else {
			dragging = false;
		}
	}

	// --- Store back relative offset ---
	TT_offset = widgetPos - scene_min;

	// Draw grey background rectangle
	draw_list->PushClipRect(scene_min, scene_max, true);
	draw_list->AddRectFilled(widgetPos, widgetPos + rect_size, IM_COL32(100, 100, 100, 188));
	draw_list->AddRect(widgetPos, widgetPos + rect_size, IM_COL32(0, 0, 0, 255));
	draw_list->PopClipRect();

	// ---- Place ImGui buttons inside rectangle ----
	ImVec2 padding(4, 4);
	float button_size = rect_size.y - padding.y * 4;
	ImVec2 start = widgetPos + padding;

	ImGui::SetCursorScreenPos(start);
	if (ImGui::ImageButton("##S",
		(ImTextureID)ContentBrowser::GetInstance().GetIcon("TRANSLATE"),
		ImVec2(button_size, button_size),
		ImVec2(0, 1),
		ImVec2(1, 0)))
	{
		gizmoOperation = ImGuizmo::TRANSLATE;
	}

	ImGui::SameLine();
	if (ImGui::ImageButton("##R",
		(ImTextureID)ContentBrowser::GetInstance().GetIcon("ROTATE"),
		ImVec2(button_size, button_size),
		ImVec2(0, 1),
		ImVec2(1, 0)))
	{
		gizmoOperation = ImGuizmo::ROTATE;
	}

	ImGui::SameLine();
	if (ImGui::ImageButton("##T",
		(ImTextureID)ContentBrowser::GetInstance().GetIcon("SCALE"),
		ImVec2(button_size, button_size),
		ImVec2(0, 1),
		ImVec2(1, 0)))
	{
		gizmoOperation = ImGuizmo::SCALE;
	}
}
