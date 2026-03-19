#include "pch.h"
#include "EventHistory.h"
#include "../AGEngine/Application.h"
#include "imgui.h"

namespace AG {
	namespace AGImGui {

		void EventHistory::Render()
		{
			// Start the ImGui window. If it's collapsed, we don't need to render its contents.
			if (!ImGui::Begin("Event History"))
			{
				ImGui::End();
				return;
			}

			// Add a button to clear the event log
			if (ImGui::Button("Clear Log")) {
				// Use the static Get() method to access the application instance
				Application::Get().ClearEventLog();
			}

			ImGui::SameLine();
			ImGui::Text("Showing last %d events.", (int)Application::Get().GetEventLog().size());

			ImGui::Separator();

			// Create a scrolling child region for the log entries
			ImGui::BeginChild("EventLogScrollingRegion");

			// Get the log from the application and display each event string
			const auto& log = Application::Get().GetEventLog();
			for (const auto& eventString : log)
			{
				ImGui::TextUnformatted(eventString.c_str());
			}

			// If the scrollbar is at the bottom, keep it there as new items are added
			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);

			ImGui::EndChild();
			ImGui::End();
		}
	}
}