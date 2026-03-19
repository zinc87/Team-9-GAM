#include "pch.h"
#include "Canvas.h"

void AG::AGImGui::Canvas::Render()
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Canvas", nullptr, windowFlags);

    // --- Canvas Setup ---
    windowPos = ImGui::GetWindowPos();
    ImVec2 availSize = ImGui::GetContentRegionAvail();

    float image_aspectR = static_cast<float>(AGWINDOW.getAspectRatio().first) /
        static_cast<float>(AGWINDOW.getAspectRatio().second);
    float window_aspectR = availSize.x / availSize.y;

    if (window_aspectR < image_aspectR) {
        imageSize.x = availSize.x;
        imageSize.y = imageSize.x / image_aspectR;
    }
    else {
        imageSize.y = availSize.y;
        imageSize.x = imageSize.y * image_aspectR;
    }

    ImVec2 cursorPos = ImGui::GetCursorPos();
    imagePos.x = cursorPos.x + (availSize.x - imageSize.x) * 0.5f;
    imagePos.y = cursorPos.y + (availSize.y - imageSize.y) * 0.5f;

    ImGui::SetCursorPos(imagePos);
    ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(AGWINDOW.getFramebuffer("Canvas"))),
        imageSize, ImVec2{ 0,1 }, ImVec2{ 1,0 });

    // --- Gizmo Setup ---
    auto selected = IMGUISYSTEM.GetSelectedObj();
    if (selected.lock())
    {
        auto trf_weak = selected.lock()->GetComponent<Component::RectTransformComponent>();
        auto selected_shared = trf_weak.lock();
        if (selected_shared)
        {
            // Same projection as BatchRenderer
            auto [width, height] = AGWINDOW.getViewportSize();
            float aspect = static_cast<float>(height) / static_cast<float>(width);
            glm::mat4 proj = glm::scale(glm::mat4(1.0f), glm::vec3(aspect, 1.0f, 1.0f));
            glm::mat4 view = glm::mat4(1.0f);

            glm::mat4 m2w = selected_shared->GetTransform().getM2W4();

            ImGuizmo::SetOrthographic(true);
            ImGuizmo::SetDrawlist();

            ImGuizmo::SetRect(windowPos.x + imagePos.x,
                windowPos.y + imagePos.y,
                imageSize.x,
                imageSize.y);

            ImGuizmo::Manipulate(glm::value_ptr(view),
                glm::value_ptr(proj),
				gizmoOperation,
                ImGuizmo::LOCAL,
                glm::value_ptr(m2w));
			float old_z = selected_shared->GetTransform().position.z;
            if (ImGuizmo::IsUsing())
            {
                glm::vec3 pos, rot, scl;
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(m2w),
                    glm::value_ptr(pos),
                    glm::value_ptr(rot),
                    glm::value_ptr(scl));

                auto& trf = selected_shared->GetTransform();
                trf.position = pos;
				trf.position.z = old_z; // z no changes
                trf.rotation = rot.z; // only Z rotation in 2D
                trf.scale = glm::vec2(scl.x, scl.y);
            }
        }
    }

    RenderTransformTool();

    ImGui::End();
}

void AG::AGImGui::Canvas::RenderTransformTool()
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

