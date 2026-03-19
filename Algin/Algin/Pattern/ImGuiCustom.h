#pragma once
#include "pch.h"

#include "../Graphics/BatchRenderer.h"
namespace ImGuiCustom
{
	bool SearchableCombo(const char* label, int* current_item, const std::vector<std::string>& items);


	void TextureCoordinatesPicker(const char* label, ImTextureID textureID, TextureCoordinate* textureCoords, float sliderSpeed = 0.005f);
}