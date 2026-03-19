#include "pch.h"
#include "ImGame.h"

std::pair<float, float> AG::AGImGui::GameView::mouseNDC = { 0.0f, 0.0f };

AG::AGImGui::GameView::~GameView()
{
}

void AG::AGImGui::GameView::Render()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("Game", nullptr, windowFlags);


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
	ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(AGWINDOW.getFramebuffer("Game"))), imageSize, ImVec2{ 0,1 }, ImVec2{ 1,0 });


	ImVec2 rectMin = ImGui::GetItemRectMin();
	ImVec2 rectSize = ImGui::GetItemRectSize();
	ImVec2 mousePos = ImGui::GetMousePos();

	// B. Normalize [0 to 1]
	float normX = (mousePos.x - rectMin.x) / rectSize.x;
	float normY = (mousePos.y - rectMin.y) / rectSize.y;

	// C. Standard NDC [-1 to 1]
	float standardNDCX = (normX * 2.0f) - 1.0f;

	// Y-Flip: ImGui (0=Top) to OpenGL (1=Top)
	float standardNDCY = 1.0f - (normY * 2.0f);

	// D. APPLY ASPECT RATIO SCALING
	// We multiply X by the Aspect Ratio.
	// Example: If 16:9, X will range from -1.77 to 1.77. Y remains -1 to 1.
	float finalX = standardNDCX * image_aspectR;
	float finalY = standardNDCY;

	mouseNDC = { finalX, finalY };
	ImGui::End();
}
