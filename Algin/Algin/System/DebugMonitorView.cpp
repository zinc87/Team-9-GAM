#include "pch.h"
#include "DebugMonitorView.h"
#include "imgui.h"

namespace AG
{
    namespace AGImGui
    {
        void DebugMonitorView::Render()
        {
            ImGui::Begin(m_name.c_str());

            // Display the key press count from the monitor
            ImGui::Text("Total Key Presses: %d", m_monitor->GetKeyPressCount());

            // Change the color of the text based on mouse state
            ImVec4 color = m_monitor->IsMouseDown() ? ImVec4(1.0f, 0.5f, 0.5f, 1.0f) : ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
            const char* text = m_monitor->IsMouseDown() ? "Mouse Button is DOWN" : "Mouse Button is UP";

            ImGui::TextColored(color, text);

            ImGui::End();
        }
    }
}
