#include "pch.h"
#include "ImGuiCustom.h"

bool ImGuiCustom::SearchableCombo(const char* label, int* current_item, const std::vector<std::string>& items)
{
    static char filter[128] = "";
    bool changed = false;

    if (ImGui::BeginCombo(label, items.empty() ? "" : items[*current_item].c_str()))
    {
        // --- Filter input ---
        ImGui::InputText("##combo_filter", filter, IM_ARRAYSIZE(filter));

        // --- List items ---
		for (int i = 0; i < static_cast<int>(items.size()); ++i)
		{
			if (std::strlen(filter) > 0)
			{
				std::string lowerItem = items[i];
				std::string lowerFilter = filter;

				auto to_lower_inplace = [](std::string& s) {
					std::transform(s.begin(), s.end(), s.begin(),
						[](unsigned char ch) -> char { return static_cast<char>(std::tolower(ch)); });
					};

				to_lower_inplace(lowerItem);
				to_lower_inplace(lowerFilter);

				if (lowerItem.find(lowerFilter) == std::string::npos)
					continue;
			}

			bool is_selected = (i == *current_item);
			if (ImGui::Selectable(items[i].c_str(), is_selected)) {
				*current_item = i;
				changed = true;
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

        ImGui::EndCombo();
    }
    return changed;
}

void ImGuiCustom::TextureCoordinatesPicker(const char* label, ImTextureID textureID, TextureCoordinate* textureCoords, float sliderSpeed)
{
	ImGui::PushID(label);
	ImGui::Text(label);

	static ImVec2 TCP_prevMousePos;
	float imageWidth = ImGui::GetColumnWidth();
	ImVec2 imageSize = ImVec2(imageWidth, imageWidth);
	ImVec2 topLeft = ImGui::GetCursorScreenPos();
	ImVec2 bottomRight = topLeft + imageSize;
	ImVec2 imageCenter = { topLeft.x + imageWidth / 2.f, topLeft.y + imageWidth / 2.f };
	float circleRadius = imageWidth / 50.f;
	float circleDiameter = circleRadius * 2.f;


	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImU32 whiteColor = ImGui::GetColorU32(ImVec4(1, 1, 1, 1));
	/*[[MAYBE_UNUSED]] ImU32 greyColor = ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.2f, 0.2f));*/

	ImVec2 points[4];

	std::pair<float, float> vertical, horizontal;
	// first = top , second = bottom 
	vertical = { textureCoords->textureCoord[0][1], textureCoords->textureCoord[2][1] };
	// first = left, second = right;
	horizontal = { textureCoords->textureCoord[0][0], textureCoords->textureCoord[2][0] };

	/* Convert Texture Coordinates to ImGui screen position points */
	for (int i = 0; i < 4; i++)
	{
		points[i] = topLeft + ImVec2(textureCoords->textureCoord[i][0] * imageWidth, (1.f - textureCoords->textureCoord[i][1]) * imageWidth);
	}

	/* Render the texture */
	ImGui::Image(textureID, imageSize, { 0, 1 }, { 1,0 }, ImVec4(1, 1, 1, 1), ImVec4(0.4f, 0.4f, 0.4f, 0.4f));
	
	//ImGui::Image(textureID, imageSize, { 0, 0 }, { 1, 1 }, ImVec4(1, 1, 1, 1), ImVec4(0.4f, 0.4f, 0.4f, 0.4f));
	
	/*[[MAYBE_UNUSED]] bool mouseInImage = ImGui::IsItemHovered();*/
	//std::cout << "mouseInImage: " << (mouseInImage ? "true" : "false") << std::endl;

	/* Render invisibile buttons for all points */
	for (int i = 0; i < 4; i++)
	{
		ImGui::SetCursorScreenPos(points[i] - ImVec2(circleRadius, circleRadius));
		std::string colorPickerPoints = "##colorPickerPoint" + i;
		ImGui::InvisibleButton(colorPickerPoints.c_str(), ImVec2(circleDiameter, circleDiameter));
		if (ImGui::IsItemActive())
		{
			ImVec2 currentMousePos = ImGui::GetMousePos();
			ImVec2 offset = currentMousePos - TCP_prevMousePos;

			switch (i)
			{
			case 0:
				vertical.first -= offset.y / imageWidth;
				horizontal.first += offset.x / imageWidth;
				break;

			case 1:
				vertical.first -= offset.y / imageWidth;
				horizontal.second += offset.x / imageWidth;
				break;
			case 2:
				vertical.second -= offset.y / imageWidth;
				horizontal.second += offset.x / imageWidth;
				break;
			case 3:
				vertical.second -= offset.y / imageWidth;
				horizontal.first += offset.x / imageWidth;
				break;
			}

		}
	}


	/* Render invisible box inside all four points */

	ImGui::SetCursorScreenPos(points[0]);
	ImVec2 boxSize = points[2] - points[0];
	ImGui::InvisibleButton("##DragBox", boxSize);
	if (ImGui::IsItemActive())
	{
		ImVec2 currentMousePos = ImGui::GetMousePos();
		ImVec2 offset = currentMousePos - TCP_prevMousePos;
		/*[[MAYBE_UNUSED]] float y_offset = offset.y / imageWidth;
		[[MAYBE_UNUSED]] float x_offset = offset.x / imageWidth;*/
		if (points[0].y + offset.y >= (imageCenter.y - imageWidth / 2.f) && points[3].y + offset.y <= (imageCenter.y + imageWidth / 2.f))
		{
			vertical.first -= offset.y / imageWidth;
			vertical.second -= offset.y / imageWidth;
		}

		if (points[0].x + offset.x >= (imageCenter.x - imageWidth / 2.f) && points[2].x + offset.x <= (imageCenter.x + imageWidth / 2.f))
		{
			horizontal.first += offset.x / imageWidth;
			horizontal.second += offset.x / imageWidth;
		}

	}


	vertical.first = std::clamp(vertical.first, 0.f, 1.f);
	vertical.second = std::clamp(vertical.second, 0.f, 1.f);

	horizontal.first = std::clamp(horizontal.first, 0.f, 1.f);
	horizontal.second = std::clamp(horizontal.second, 0.f, 1.f);

	textureCoords->textureCoord[0][0] = horizontal.first;
	textureCoords->textureCoord[0][1] = vertical.first;

	textureCoords->textureCoord[1][0] = horizontal.second;
	textureCoords->textureCoord[1][1] = vertical.first;

	textureCoords->textureCoord[2][0] = horizontal.second;
	textureCoords->textureCoord[2][1] = vertical.second;

	textureCoords->textureCoord[3][0] = horizontal.first;
	textureCoords->textureCoord[3][1] = vertical.second;

	/* Draw 4 points and connecting lines */
	for (int i = 0; i < 4; i++)
	{
		ImVec2 point1 = points[i];
		ImVec2 point2 = points[i + 1 > 3 ? 0 : i + 1];
		drawList->AddCircle(point1, 2, whiteColor, i + 4, circleRadius);
		drawList->AddLine(point1, point2, whiteColor, circleRadius / 2.f);
	}

	ImGui::SetCursorScreenPos(bottomRight);
	ImGui::NewLine();
	ImGui::Columns(2);
	float columnWidth = ImGui::GetColumnWidth();
	ImVec2 itemSpacing = ImGui::GetStyle().ItemSpacing;
	float sliderWidth = columnWidth / 2.f - itemSpacing.x;

	ImVec2 wordSpace = ImGui::CalcTextSize("Top Left");
	float startingPoint = ImGui::GetCursorPosX();

	ImGui::SetCursorPosX(startingPoint + (columnWidth - wordSpace.x) / 2.f);
	ImGui::Text("Top Left");
	ImGui::SetNextItemWidth(sliderWidth);
	ImGui::DragFloat("##TCP_TL_x", &textureCoords->textureCoord[0][0], sliderSpeed);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(sliderWidth);
	ImGui::DragFloat("##TCP_TL_y", &textureCoords->textureCoord[0][1], sliderSpeed);
	ImGui::NextColumn();

	startingPoint = ImGui::GetCursorPosX();
	ImGui::SetCursorPosX(startingPoint + (columnWidth - wordSpace.x) / 2.f);
	ImGui::Text("Top Right");
	ImGui::SetNextItemWidth(sliderWidth);
	ImGui::DragFloat("##TCP_TR_x", &textureCoords->textureCoord[1][0], sliderSpeed);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(sliderWidth);
	ImGui::DragFloat("##TCP_TR_y", &textureCoords->textureCoord[1][1], sliderSpeed);
	ImGui::NextColumn();

	startingPoint = ImGui::GetCursorPosX();
	ImGui::SetCursorPosX(startingPoint + (columnWidth - wordSpace.x) / 2.f);
	ImGui::Text("Bot Right");
	ImGui::SetNextItemWidth(sliderWidth);
	ImGui::DragFloat("##TCP_BR_x", &textureCoords->textureCoord[2][0], sliderSpeed);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(sliderWidth);
	ImGui::DragFloat("##TCP_BR_y", &textureCoords->textureCoord[2][1], sliderSpeed);
	ImGui::NextColumn();

	startingPoint = ImGui::GetCursorPosX();
	ImGui::SetCursorPosX(startingPoint + (columnWidth - wordSpace.x) / 2.f);
	ImGui::Text("Bot Left");
	ImGui::SetNextItemWidth(sliderWidth);
	ImGui::DragFloat("##TCP_BL_x", &textureCoords->textureCoord[3][0], sliderSpeed);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(sliderWidth);
	ImGui::DragFloat("##TCP_BL_y", &textureCoords->textureCoord[3][1], sliderSpeed);
	ImGui::NextColumn();




	TCP_prevMousePos = ImGui::GetMousePos();
	ImGui::PopID();
	ImGui::Columns(1);
}
