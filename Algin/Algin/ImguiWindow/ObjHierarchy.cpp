/*!*****************************************************************************
\file ObjHierarchy.cpp
\author Zulfami Ashrafi Bin Wakif
\date 20/1/2025 (MM/DD/YYYY)

\brief obj hierarchy for the engine
*******************************************************************************/
#pragma once
#include "pch.h"

#include "../Object Base/Prefab/PrefabEditModeController.h"

namespace AG {
	namespace AGImGui {
		ObjHierarchy::~ObjHierarchy() {

		}
		void AddGameObjPopUpContent(const std::shared_ptr<System::IObject>& parent = NULL) {
			if (ImGui::MenuItem("Empty GameObject"))
			{
				/*auto cmd = std::make_unique<System::CreateObjectCommand>();
				Pattern::CommandManager::GetInstance().ExecuteCommand(std::move(cmd));*/

				auto m_object = System::ObjectManager::GetInstance().CreateObject("", true, true);
				COMPONENTMANAGER.CreateComponent(m_object.get(), Data::ComponentTypes::Transform);

				if (parent != NULL) {
					OBJECTMANAGER.LinkParentChild(parent, m_object);
				}
			}


			if (ImGui::BeginMenu("UI")) {
				if (ImGui::MenuItem("Image"))
				{
					auto m_object = System::ObjectManager::GetInstance().CreateObject("", true, true);
					COMPONENTMANAGER.CreateComponent(m_object.get(), Data::ComponentTypes::RectTransform);
					COMPONENTMANAGER.CreateComponent(m_object.get(), Data::ComponentTypes::Image2DComponent);

					if (parent != NULL) {
						OBJECTMANAGER.LinkParentChild(parent, m_object);
					}
				}

				if (ImGui::MenuItem("Text"))
				{
					auto m_object = System::ObjectManager::GetInstance().CreateObject("", true, true);
					COMPONENTMANAGER.CreateComponent(m_object.get(), Data::ComponentTypes::RectTransform);
					COMPONENTMANAGER.CreateComponent(m_object.get(), Data::ComponentTypes::TextMeshUI);
					if (parent != NULL) {
						OBJECTMANAGER.LinkParentChild(parent, m_object);
					}
				}
				ImGui::EndMenu();
			}


			if (ImGui::MenuItem("Light"))
			{
				auto m_object = System::ObjectManager::GetInstance().CreateObject("", true, true);
				COMPONENTMANAGER.CreateComponent(m_object.get(), Data::ComponentTypes::Transform);
				COMPONENTMANAGER.CreateComponent(m_object.get(), Data::ComponentTypes::Light);
				if (parent != NULL) {
					OBJECTMANAGER.LinkParentChild(parent, m_object);
				}
			}

			if (ImGui::MenuItem("Camera"))
			{
				auto m_object = System::ObjectManager::GetInstance().CreateObject("", true, true);
				COMPONENTMANAGER.CreateComponent(m_object.get(), Data::ComponentTypes::Transform);
				COMPONENTMANAGER.CreateComponent(m_object.get(), Data::ComponentTypes::Camera);
				if (parent != NULL) {
					OBJECTMANAGER.LinkParentChild(parent, m_object);
				}
			}

			if (ImGui::MenuItem("Audio"))
			{
				auto m_object = System::ObjectManager::GetInstance().CreateObject("", true, true);
				COMPONENTMANAGER.CreateComponent(m_object.get(), Data::ComponentTypes::Transform);
				COMPONENTMANAGER.CreateComponent(m_object.get(), Data::ComponentTypes::Audio);
				if (parent != NULL) {
					OBJECTMANAGER.LinkParentChild(parent, m_object);
				}
			}
		}

		void ObjHierarchy::Render() {
			ImGui::SetNextWindowSize(ImVec2(m_size.first, m_size.second), ImGuiCond_FirstUseEver);

			ImGui::Begin(m_name.c_str());

			static char searchBuffer[128] = "";
            static bool showAddPopup = false;

            auto& objMgr = System::ObjectManager::GetInstance();

            // --- Search Bar ---
            ImGui::Text("Search:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200);
            ImGui::InputText("##Search", searchBuffer, IM_ARRAYSIZE(searchBuffer));

            // --- "+" Button ---
            ImGui::SameLine();
            if (ImGui::Button("+"))
            {
                showAddPopup = true;
                ImGui::OpenPopup("Add GameObject");
            }

            // --- Add GameObject Popup ---
            // TODO: Make this come from factory
            if (ImGui::BeginPopup("Add GameObject"))
            {
				AddGameObjPopUpContent();
                ImGui::EndPopup();
            }
            ImGui::Separator();


			ImGui::BeginChild("ScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
            // --- Header for the current scene ---
            DisplayObjectTree(objMgr.GetRoot(), std::string(searchBuffer));
			ImGui::EndChild();

			ImGui::End();
		}
		
		// Search helper
		bool MatchesSearch(const std::string& name, const std::string& filter)
		{
			if (filter.empty()) return true;

			std::string lowerName = name;
			std::string lowerFilter = filter;

			auto to_lower_char = [](unsigned char ch) -> char {
				return static_cast<char>(std::tolower(ch));
				};

			std::ranges::transform(lowerName, lowerName.begin(), to_lower_char);
			std::ranges::transform(lowerFilter, lowerFilter.begin(), to_lower_char);

			return lowerName.find(lowerFilter) != std::string::npos;
		}

		bool HasMatchingDescendant(const std::shared_ptr<System::IObject>& object, const std::string& filter)
		{
			for (const auto& child : object->GetChildren())
			{
				if (MatchesSearch(child->GetName(), filter) || HasMatchingDescendant(child, filter))
					return true;
			}
			return false;
		}
		// Search helper

		void ObjHierarchy::DisplayObjectTree(const std::shared_ptr<System::IObject>& object, const std::string& filterText)
		{
			if (!object) return;

			// Skip rendering the invisible root
			if (object->GetName() == "__root__")
			{
				for (const auto& child : object->GetChildren())
				{
					DisplayObjectTree(child, filterText);
				}
				return;
			}

			// === Skip non-matching nodes unless they have matching children ===
			bool selfMatches = MatchesSearch(object->GetName(), filterText);
			bool showNode = selfMatches || HasMatchingDescendant(object, filterText);
			if (!showNode) return;

			auto& objMgr = System::ObjectManager::GetInstance();
			auto& guiSystem = System::ImGuiSystem::GetInstance();

			bool isRenaming = (m_renamingObject == object);
			std::string label = isRenaming ? "###RenamingNode" : object->GetName() + "###" + Data::GUIDShort(object->GetID());

			// === Build TreeNode flags ===
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_OpenOnDoubleClick |
				ImGuiTreeNodeFlags_SpanFullWidth;

			if (isRenaming) 
				flags |= ImGuiTreeNodeFlags_Selected;

			if (object->GetChildren().empty())
				flags |= ImGuiTreeNodeFlags_Leaf;

			if (object == guiSystem.GetSelectedObj().lock()) // Built-in selection highlighting
				flags |= ImGuiTreeNodeFlags_Selected;

			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			bool opened = ImGui::TreeNodeEx(label.c_str(), flags);


			// === Handle Context Menu ===
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && !PrefabEditModeController::GetInstance().isActive())
			{
				ImGui::OpenPopup(("RightClickMenu_" + Data::GUIDShort(object->GetID())).c_str());
			}

			if (ImGui::BeginPopupContextItem(("RightClickMenu_" + Data::GUIDShort(object->GetID())).c_str()))
			{
				if (ImGui::BeginMenu("Add")) {
					AddGameObjPopUpContent(object);
					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Cut"))
				{
					// TODO: Cut Feature
				}

				if (ImGui::MenuItem("Copy"))
				{
					// TODO: Copy Feature
				}

				if (ImGui::MenuItem("Paste"))
				{
					// TODO: Paste Feature
				}

				if (ImGui::MenuItem("Paste as Child"))
				{
					// TODO: Paste as Child Feature
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Create Prefab From This (To be Fixed)")) {
					//PREFABMANAGER.CreatePrefabTemplate(object);
				}

				if (ImGui::MenuItem("Rename"))
				{
					m_renamingObject = object;
					strncpy_s(m_renameBuffer, object->GetName().c_str(), sizeof(m_renameBuffer));
					m_renameBuffer[sizeof(m_renameBuffer) - 1] = '\0';
					m_renameJustStarted = true;
				}

				if (ImGui::MenuItem("Duplicate"))
				{
					objMgr.DuplicateObject(object);
				}

				if (ImGui::MenuItem("Delete"))
				{
					auto cmd = std::make_unique<System::DeleteObjectCommand>(object);
					Pattern::CommandManager::GetInstance().ExecuteCommand(std::move(cmd));
				}

				ImGui::EndPopup();
			}


			// === Handle selection click ===
			if (ImGui::IsItemHovered()
				&& ImGui::IsMouseReleased(ImGuiMouseButton_Left)
				&& ImGui::GetDragDropPayload() == nullptr)
			{
				System::ImGuiSystem::GetInstance().SetSelectedObj(object);
				ObjInspector::CloseInspector();
			}

			// === Handle Renaming ===
			if (isRenaming)
			{
				// Get the rectangle where the label was drawn
				ImVec2 labelMin = ImGui::GetItemRectMin();
				ImVec2 labelMax = ImGui::GetItemRectMax();

				// Move the cursor to that position
				ImGui::SetCursorScreenPos(labelMin);

				// Calculate exact width to match label area
				float width = labelMax.x - labelMin.x;
				ImGui::SetNextItemWidth(width);

				ImGuiInputTextFlags inputFlags =
					ImGuiInputTextFlags_EnterReturnsTrue |
					ImGuiInputTextFlags_AutoSelectAll;

				// Autofocus when renaming starts
				if (m_renameJustStarted)
				{
					ImGui::SetKeyboardFocusHere();
					m_renameJustStarted = false;
				}

				// Actual input field (drawn on top of label)
				if (ImGui::InputText("##RenameField", m_renameBuffer, IM_ARRAYSIZE(m_renameBuffer), inputFlags))
				{
					object->SetName(m_renameBuffer);
					m_renamingObject = nullptr;
				}

				// Commit when clicking outside (but not hovering menus etc.)
				if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0))
				{
					object->SetName(m_renameBuffer);
					m_renamingObject = nullptr;
				}

				// Cancel with Escape
				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				{
					m_renamingObject = nullptr;
				}
			}

			// === Drag Source ===
			if (ImGui::BeginDragDropSource())
			{
				const std::string& id = object->GetID();
				ImGui::SetDragDropPayload("DND_OBJECT", id.c_str(), id.size() + 1); // include null terminator
				ImGui::Text("Move: %s", object->GetName().c_str());
				ImGui::EndDragDropSource();
			}

			// === Drop Target ON the TreeNode ===
			if (ImGui::BeginDragDropTarget())
			{
				ImVec2 min = ImGui::GetItemRectMin();
				ImVec2 max = ImGui::GetItemRectMax();
				float height = max.y - min.y;
				float mouseY = ImGui::GetMousePos().y;

				float topZone = min.y + height * 0.4f;
				float bottomZone = max.y - height * 0.4f;

				// Drop zone visuals
				if (mouseY < topZone)
					ImGui::GetWindowDrawList()->AddRectFilled(min, ImVec2(max.x, min.y + 1.0f), IM_COL32(255, 255, 0, 255));
				else if (mouseY > bottomZone)
					ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x, max.y - 1.0f), max, IM_COL32(255, 255, 0, 255));
				else
					ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(80, 80, 255, 60)); // highlight full node for reparent

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_OBJECT"))
				{
					const char* draggedID = static_cast<const char*>(payload->Data);
					auto draggedObj = objMgr.GetObjectByID(draggedID);

					if (draggedObj && draggedObj != object)
					{
						// --- Prevent dragging a parent into its own descendant ---
						// Do this check ONCE at the top.
						auto descendants = draggedObj->GetDescendants();
						bool isDescendant = std::find(descendants.begin(), descendants.end(), object) != descendants.end();

						// If it's an illegal move, just stop.
						if (isDescendant)
						{
							// You could add feedback here, like changing the cursor
						}
						// Otherwise, the move is legal, so proceed
						else
						{
							auto targetParent = object->GetParent();
							auto draggedParent = draggedObj->GetParent();

							if (targetParent)
							{
								auto siblings = targetParent->GetChildren();
								auto it = std::find(siblings.begin(), siblings.end(), object);
								size_t index = std::distance(siblings.begin(), it);

								if (mouseY < topZone)
								{
									objMgr.LinkParentChild(targetParent, draggedObj);
									objMgr.ReorderChild(targetParent, draggedObj, index);
								}
								else if (mouseY > bottomZone)
								{
									objMgr.LinkParentChild(targetParent, draggedObj);
									objMgr.ReorderChild(targetParent, draggedObj, index + 1);
								}
								else
								{
									// Reparent to object (middle zone)
									// We already know it's not a descendant, so just link it.
									objMgr.LinkParentChild(object, draggedObj);
								}
							}
							else
							{
								// fallback reparent
								// We also know this is safe.
								objMgr.LinkParentChild(object, draggedObj);
							}
						}
					}
				}

				ImGui::EndDragDropTarget();
			}

			// === Using Key Bind ===
			if (ImGui::IsKeyPressed(ImGuiKey_F2) && guiSystem.GetSelectedObj().lock())
			{
				// Start renaming the selected object
				
				m_renamingObject = guiSystem.GetSelectedObj().lock();

				// Copy current name into rename buffer
				strncpy_s(m_renameBuffer, guiSystem.GetSelectedObj().lock()->GetName().c_str(), sizeof(m_renameBuffer));
				m_renameBuffer[sizeof(m_renameBuffer) - 1] = '\0';

				m_renameJustStarted = true; // trigger auto-focus
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Delete) && guiSystem.GetSelectedObj().lock())
			{
				auto cmd = std::make_unique<System::DeleteObjectCommand>(guiSystem.GetSelectedObj().lock());
				Pattern::CommandManager::GetInstance().ExecuteCommand(std::move(cmd));
				guiSystem.SetSelectedObj(nullptr);
			}

			// === Recurse ===
			if (opened)
			{
				for (const auto& child : object->GetChildren())
				{
					DisplayObjectTree(child, filterText);
				}
				ImGui::TreePop();
			}
		}
	}
}