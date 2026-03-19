/************************************************************************/
/*!
	\file   ConsoleIMGUI.cpp
	\author Bin Wakif Zulfami Ashrafi, b.zulfamiashrafi, 2301298
	\email  b.zulfamiashrafi@digipen.edu
	\date   8 November 2024
	\brief  This file contains definitions for ConsoleIMGUI to display 
			the debug console.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/************************************************************************/
#include "pch.h"

namespace AG {
	namespace AGImGui {
		void Console::Render() {
			if (ImGui::Begin("Console")) {
				std::shared_ptr<System::ImGuiLogger> logger = LOGGER.GetImGuiLogger();
				std::shared_ptr<System::ImGuiLogger> inputLogger = LOGGER.GetImGuiInputLogger();
				std::shared_ptr<System::ImGuiLogger> graphicLogger = LOGGER.GetImGuiGraphicsLogger();

				// Input Logger =====================================================================//
				ImGui::BeginGroup();
				if (ImGui::BeginTabBar("Specialize_Logger")) {

					if (ImGui::BeginTabItem("Input Logger")) {
						ImGui::Text("Input Logger");
						ImGui::SameLine();
						if (ImGui::Button("Clear##InputLogger")) {
							inputLogger->ClearLog();
						}
						ImGui::SameLine();
						ImGui::Checkbox("Auto-scroll##input", inputLogger->GetScrollPtr());
						ImGui::Separator();

						ImGui::BeginChild("Input", ImVec2(300, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
						inputLogger->Draw();
						ImGui::EndChild();
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Graphics_Logger")) {
						ImGui::Text("Graphics Logger");
						ImGui::SameLine();
						if (ImGui::Button("Clear##InputLogger")) {
							graphicLogger->ClearLog();
						}
						ImGui::SameLine();
						ImGui::Checkbox("Auto-scroll##input", graphicLogger->GetScrollPtr());
						ImGui::Separator();

						ImGui::BeginChild("Graphics", ImVec2(300, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
						graphicLogger->Draw();
						ImGui::EndChild();
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Obj/Comp List"))
					{
						ImGui::Text("All Objects and Components");
						ImGui::Separator();

						ImGui::BeginChild("ObjCompChild", ImVec2(300, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

						auto& objMgr = System::ObjectManager::GetInstance();
						const auto& objects = objMgr.GetAllObjects();

						// Draw all obj
						ImGui::Text("Object in the map");
						for (const auto&[id, obj] : objects)
						{
							if (!obj) continue;
							if (ImGui::TreeNode(obj->Name().c_str())) {
								ImGui::BulletText("Obj ID: %s", obj->GetID().c_str());
								ImGui::BulletText("Obj Parent: %s", obj->GetParent()->GetName().c_str());

								if (ImGui::TreeNode("Children List")) {
									const auto& childs = obj->GetChildren();
									for (const auto& child : childs) {
										ImGui::BulletText("%s", child->GetName().c_str());
									}
									ImGui::TreePop();
								}
								ImGui::TreePop();
							}
						}

						auto& compMgr = System::ComponentManager::GetInstance();
						const auto& components = compMgr.GetComponents();

						// Draw all comp
						ImGui::Text("Component in the map");
						for (const auto&[id, compPtr] : components)
						{
							ImGui::BulletText("%s", compPtr->GetTypeName().c_str());

						}
						ImGui::EndChild();
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				ImGui::EndGroup();

				ImGui::SameLine();

				// Main Logger =======================================================================//
				ImGui::BeginGroup();
				ImGui::Text("Main Console Logger");
				ImGui::SameLine();
				if (ImGui::Button("Clear##MainLogger")) {
					logger->ClearLog();
				}
				ImGui::SameLine();
				ImGui::Checkbox("Auto-scroll##Console", logger->GetScrollPtr());
				ImGui::Separator();

				ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
				logger->Draw();
				ImGui::EndChild();
				ImGui::EndGroup();
			}
			ImGui::End();
		}
	}
}