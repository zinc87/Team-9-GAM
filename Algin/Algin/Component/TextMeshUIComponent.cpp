#include "pch.h"
#include "TextMeshUIComponent.h"

void AG::Component::TextMeshUIComponent::Awake()
{
}

void AG::Component::TextMeshUIComponent::Start()
{
}

void AG::Component::TextMeshUIComponent::Update()
{
}

void AG::Component::TextMeshUIComponent::LateUpdate()
{
	if (PIPELINE.GetPipeline() == RenderPipeline::STATE::BATCH)
	{
		if (font_hash != 0) // render if there is font selected
		{
			auto attachedObj = GetObj().lock();
			if (!attachedObj) return;

			auto trf = attachedObj->GetComponent<Component::RectTransformComponent>().lock();
			if (trf)
			{
				// Copy transform and bump Z slightly so text always renders
				// in front of images at the same nominal Z position.
				// This prevents component iteration order from affecting
				// text vs. image draw order in the batch renderer.
				auto textTransform = trf->GetTransform();
				textTransform.position.z += 0.001f;
				BatchRenderer::GetInstance().RenderText(text_content, font_color, textTransform, font_hash, wrap,wrap_limit, left,centered,right, font_size, line_spacing, letter_spacing);
			}

		}
	}
}

void AG::Component::TextMeshUIComponent::Free()
{
}

void AG::Component::TextMeshUIComponent::Inspector()
{
	float x_width = ImGui::GetContentRegionAvail().x;


	auto fontAsset = AssetManager::GetInstance().GetAssets(ASSET_TYPE::AGFONT);
	std::vector<const char*> fontNames;
	fontNames.push_back("No Font");
	for (auto& [name, asset] : fontAsset)
	{
		if (asset.lock())
		{
			fontNames.push_back(asset.lock()->asset_name.c_str());
		}
	}

	int m_fontIndex = 0; // default to "No Mesh"
	for (int i = 1; i < (int)fontNames.size(); ++i)
	{
		if (font_hash == HASH(fontNames[i])) { m_fontIndex = i; break; }
	}

	ImGui::Text("Font Style"); ImGui::SameLine(); ImGui::SetCursorPosX(x_width * 0.3f);
	if (ImGui::Combo("##font", &m_fontIndex, fontNames.data(), static_cast<int>(fontNames.size()) ))
	{
		if (m_fontIndex == 0)
		{
			font_hash = 0;
		}
		else
		{
			font_hash = HASH(fontNames[m_fontIndex]);
		}
	}

	ImGui::Text("Font Size"); ImGui::SameLine(); ImGui::SetCursorPosX(x_width * 0.3f);
	ImGui::DragFloat("##font_size", &font_size, 0.05f, 0.05f, 32.f);

	ImGui::Text("Line Spacing"); ImGui::SameLine(); ImGui::SetCursorPosX(x_width * 0.3f);
	ImGui::DragFloat("##line_spacing", &line_spacing, 0.05f, 1.f, 32.f);

	ImGui::Text("Letter Spacing"); ImGui::SameLine(); ImGui::SetCursorPosX(x_width * 0.3f);
	ImGui::DragFloat("##letter_spacing", &letter_spacing, 0.001f, 0.f, 2.f);

	ImGui::Text("Wrap"); ImGui::SameLine(); ImGui::SetCursorPosX(x_width * 0.3f);
	ImGui::Checkbox("##wrap", &wrap);

	ImGui::Text("Wrap Limit"); ImGui::SameLine(); ImGui::SetCursorPosX(x_width * 0.3f);
	ImGui::DragFloat("##wrap_limit", &wrap_limit, 0.01f, 0.02f, 2.f);

	ImGui::Columns(3, nullptr, false);

	// Column 1
	ImGui::Text("Left");
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::GetFrameHeight()) * 0.5f);
	ImGui::Checkbox("##align_left", &left);
	ImGui::NextColumn();

	// Column 2
	ImGui::Text("Center");
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::GetFrameHeight()) * 0.5f);
	ImGui::Checkbox("##align_center", &centered);
	ImGui::NextColumn();

	// Column 3
	ImGui::Text("Right");
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::GetFrameHeight()) * 0.5f);
	ImGui::Checkbox("##align_right", &right);

	ImGui::Columns(1);

	ImGui::ColorPicker4("##font_color", font_color.rgba_f);

	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Content").x) * 0.5f);
	ImGui::Text("Content");
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::InputTextMultiline("##Content", &text_content);

}

void AG::Component::TextMeshUIComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
	ASSIGNFROM_FN_BODY(
		BASIC_ASSIGN(font_color);
	BASIC_ASSIGN(font_hash);
	BASIC_ASSIGN(font_size);
	BASIC_ASSIGN(line_spacing);
	BASIC_ASSIGN(letter_spacing);  
	BASIC_ASSIGN(wrap_limit);       
	BASIC_ASSIGN(wrap);
	BASIC_ASSIGN(centered);
	BASIC_ASSIGN(left);
	BASIC_ASSIGN(right);
	BASIC_ASSIGN(text_content);
		)
}

void AG::Component::TextMeshUIComponent::SetFont(std::string font_str)
{
	font_hash = HASH(font_str);
}

void AG::Component::TextMeshUIComponent::SetFontSize(float size)
{
	font_size = size;
}

void AG::Component::TextMeshUIComponent::SetLineSpacing(float spacing)
{
	line_spacing = spacing;
}

void AG::Component::TextMeshUIComponent::SetLetterSpacing(float spacing)
{
	letter_spacing = spacing;
}

void AG::Component::TextMeshUIComponent::SetWrapLimit(float limit)
{
	wrap_limit = limit;
}

void AG::Component::TextMeshUIComponent::SetWrap(bool wrap)
{
	this->wrap = wrap;
}

void AG::Component::TextMeshUIComponent::SetCentered(bool centered)
{
	this->centered = centered;
}

void AG::Component::TextMeshUIComponent::SetColor(float r, float g, float b, float a)
{
	this->font_color = Color(r, g, b, a);
}


REGISTER_REFLECTED_TYPE(AG::Component::TextMeshUIComponent)