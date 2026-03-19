#include "pch.h"
#include "Image2DComponent.h"

void AG::Component::Image2DComponent::Awake()
{
}

void AG::Component::Image2DComponent::Start()
{
}

void AG::Component::Image2DComponent::Update()
{
}

void AG::Component::Image2DComponent::LateUpdate()
{
	//AG_CORE_INFO("Draw Quad");
	// add image to batch
	if (PIPELINE.GetPipeline() == RenderPipeline::STATE::BATCH)
	{
		auto attachedObj = GetObj().lock();
		if (!attachedObj) return;

		auto trf = attachedObj->GetComponent<Component::RectTransformComponent>().lock();
		if (trf)
		{
			BatchRenderer::GetInstance().DrawQuad(m_color, trf->GetTransform(), m_image_hashed, m_tc);
		}
	}
}

void AG::Component::Image2DComponent::Free()
{
}

void AG::Component::Image2DComponent::Inspector()
{
	// texture
	auto texAssets = AssetManager::GetInstance().GetAssets(DDS);

	std::vector<const char*> texNames;          // make texture names list
	texNames.push_back("No Image");
	for (auto& [name, asset] : texAssets)
	{
		if (asset.lock())
		{
			texNames.push_back(asset.lock()->asset_name.c_str());
		}
	}

	// Find the name of the currently selected image for the preview box
	std::string currentImageName = "No Image";
	bool found = false;

	// Check if our hash matches any loaded asset
	if (m_image_hashed != 0) {
		for (const auto& name : texNames) {
			if (m_image_hashed == HASH(name)) {
				currentImageName = name;
				found = true;
				break;
			}
		}
	}

	static ImGuiTextFilter filter;

	// set m_image_hashed on selection (SEARCHABLE)
	if (ImGui::BeginCombo("Image", currentImageName.c_str()))
	{
		// Search Bar
		filter.Draw("##ImageSearch", -1.0f);
		ImGui::Separator();

		for (int i = 0; i < (int)texNames.size(); ++i)
		{
			// Apply filter
			if (filter.PassFilter(texNames[i]))
			{
				bool isSelected = (currentImageName == texNames[i]);
				if (ImGui::Selectable(texNames[i], isSelected))
				{
					if (i == 0) m_image_hashed = 0; // "No Image" index
					else m_image_hashed = HASH(texNames[i]);
				}

				if (isSelected) ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("IMGUI_CONTENT"))
		{
			m_image_hashed = *(const size_t*)payload->Data;
		}
		ImGui::EndDragDropTarget();
	}

	// pickers
	if (!m_image_hashed)
	{
		ImGui::ColorPicker4("Color Picker", m_color.rgba_f);
	}
	else
	{
		auto assetWeak = AssetManager::GetInstance().GetAsset(m_image_hashed);
		if (auto assetShared = assetWeak.lock())
		{
			auto texAsset = std::dynamic_pointer_cast<TextureAsset>(assetShared);
			if (texAsset && texAsset->textureID != 0)
			{
				static bool ar_lock;
				ImGui::Checkbox("Lock Aspect Ratio", &ar_lock);
				if (ar_lock)
				{
					static bool locked_x;
					ImGui::Checkbox("Lock X", &locked_x);
					bool locked_y = !locked_x;
					ImGui::Checkbox("Lock Y", &locked_y);
					locked_x = !locked_y;
					float AR = (float)texAsset->size.first / (float)texAsset->size.second; // width / height 


					ImGui::Text("Original Image Dimension: %d , %d", texAsset->size.first, texAsset->size.second);
					ImGui::Text("Aspect Ratio: %f", AR);
					auto attachedObj = GetObj().lock();
					auto trf = attachedObj->GetComponent<Component::RectTransformComponent>().lock();
					if (trf)
					{
						if (locked_x)
						{
							trf->GetTransform().scale.x = trf->GetTransform().scale.y * AR;
						}
						else //locked y
						{
							trf->GetTransform().scale.y = trf->GetTransform().scale.x / AR;
						}
					}
				}


				ImGuiCustom::TextureCoordinatesPicker(
					"Texture Coordinate Picker",
					(ImTextureID)(uintptr_t)texAsset->textureID,
					&m_tc);

				ImGui::ColorPicker4("Fallback Color", m_color.rgba_f);
			}
			else
			{
				ImGui::ColorPicker4("Fallback Color", m_color.rgba_f);
			}
		}
		else
		{
			ImGui::ColorPicker4("##colorpicker2dimage", m_color.rgba_f);
		}
	}
}

void AG::Component::Image2DComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
	ASSIGNFROM_FN_BODY(
		this->m_image_hashed = fromCmpPtr->m_image_hashed;
	this->m_tc = fromCmpPtr->m_tc;
	this->m_color = fromCmpPtr->m_color;
		)
}

REGISTER_REFLECTED_TYPE(AG::Component::Image2DComponent)