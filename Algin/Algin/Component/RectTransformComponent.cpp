#include "pch.h"
#include "RectTransformComponent.h"

void AG::Component::RectTransformComponent::Awake()
{
}

void AG::Component::RectTransformComponent::Start()
{
}

void AG::Component::RectTransformComponent::Update()
{
}

void AG::Component::RectTransformComponent::LateUpdate()
{
}

void AG::Component::RectTransformComponent::Free()
{
}

void AG::Component::RectTransformComponent::Inspector()
{
    ImVec2 itemSpacing = ImGui::GetStyle().ItemSpacing;
    float frameWidth = ImGui::GetContentRegionAvail().x;
    float totalSpacing = itemSpacing.x * 2;
    float cellWidth = (frameWidth - totalSpacing) / 3.0f;

    // ---- Position ----
    {
        float totalInputsWidth = cellWidth * 3.0f + itemSpacing.x * 2.0f;
        float textWidth = ImGui::CalcTextSize("Position").x;
        float cursorX = (frameWidth - totalInputsWidth) * 0.5f
            + (totalInputsWidth - textWidth) * 0.5f;

        ImGui::SetCursorPosX(cursorX);
        ImGui::Text("Position");

        ImGui::SetNextItemWidth(cellWidth);
        ImGui::DragFloat("##PosX", &m_trf.position.x);
        ImGui::SameLine();

        ImGui::SetNextItemWidth(cellWidth);
        ImGui::DragFloat("##PosY", &m_trf.position.y);
        ImGui::SameLine();

        ImGui::SetNextItemWidth(cellWidth);
        ImGui::DragFloat("##PosZ", &m_trf.position.z, 0.01f, -1.f, 0.f);
    }

    // ---- Scale ----
    {
        float totalInputsWidth = cellWidth * 2.0f + itemSpacing.x;
        float textWidth = ImGui::CalcTextSize("Scale").x;
        float cursorX = (frameWidth - totalInputsWidth) * 0.5f
            + (totalInputsWidth - textWidth) * 0.5f;

        ImGui::SetCursorPosX(cursorX);
        ImGui::Text("Scale");

        ImGui::SetNextItemWidth(cellWidth);
        ImGui::DragFloat("##ScaleX", &m_trf.scale.x, 0.01f, 0.0f, FLT_MAX);
        ImGui::SameLine();

        ImGui::SetNextItemWidth(cellWidth);
        ImGui::DragFloat("##ScaleY", &m_trf.scale.y, 0.01f, 0.0f, FLT_MAX);
    }

    // ---- Rotation ----
    {
        float totalInputsWidth = cellWidth;
        float textWidth = ImGui::CalcTextSize("Rotation (deg)").x;
        float cursorX = (frameWidth - totalInputsWidth) * 0.5f
            + (totalInputsWidth - textWidth) * 0.5f;

        ImGui::SetCursorPosX(cursorX);
        ImGui::Text("Rotation (deg)");

        ImGui::SetNextItemWidth(cellWidth);
        ImGui::DragFloat("##Rotation", &m_trf.rotation, 0.5f, -180.0f, 180.0f);
    }
}

void AG::Component::RectTransformComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
    ASSIGNFROM_FN_BODY(
        this->m_trf = fromCmpPtr->m_trf;
    )
}
REGISTER_REFLECTED_TYPE(AG::Component::RectTransformComponent)