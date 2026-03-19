/*!*****************************************************************************
\file ImGuiSystem.cpp
\author Zulfami Ashrafi Bin Wakif
\date 19/2/2025 (MM/DD/YYYY)

\brief Manager imgui
*******************************************************************************/
#pragma once
#include "pch.h"
#include "EventHistory.h"
#include "DebugMonitorView.h"
#include "../Object Base/Prefab/PrefabEditModeController.h"
#include "VideoPlayerWindow.h"

namespace AG {
	namespace System {

		// List of all added Windows //
		void ImGuiSystem::ImguiWindowList() {
			AddWindow(std::make_unique<AGImGui::ToolBarImgui>());
			AddWindow(std::make_unique<AGImGui::ObjHierarchy>());
			AddWindow(std::make_unique<AGImGui::ObjInspector>());
			AddWindow(std::make_unique<AGImGui::SceneView>());
			AddWindow(std::make_unique<AGImGui::GameView>());
			AddWindow(std::make_unique<AGImGui::ContentBrowser>());
			AddWindow(std::make_unique<AGImGui::Console>());
			AddWindow(std::make_unique<AGImGui::Profiler>());
			AddWindow(std::make_unique<AGImGui::Canvas>());
			AddWindow(std::make_unique<AGImGui::BHTEditor>());
			
			// New Video Player Window
			AddWindow(std::make_unique<AGImGui::VideoPlayerWindow>());

			//AddWindow(std::make_unique<AGImGui::EventHistory>());
		}
		// List of all added Windows //


		ImGuiSystem::ImGuiSystem() { Init(); }
		ImGuiSystem::~ImGuiSystem() {  }

		void ImGuiSystem::Init() 
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImNodes::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;

			// When we get font
			io.Fonts->AddFontFromFileTTF("Assets/Font/Exo2-Regular.ttf", 15.0f); // Adjust the font size as needed
			io.Fonts->Build(); // Rebuild the font atlas 
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
			//UnityDarkStyle();
			UnityDarkStyle();
			ImGui_ImplGlfw_InitForOpenGL(AGWINDOW.getWindowContext(), true);
			ImGui_ImplOpenGL3_Init("#version 450");

			ImguiWindowList();
		}

		void ImGuiSystem::Free() {

			ImGui::SaveIniSettingsToDisk("../../../Algin/imgui.ini");
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImNodes::DestroyContext();
			ImGui::DestroyContext();
		}

		void ImGuiSystem::PreRender() {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			/* ---- ImGuizmo ---- */
			ImGuizmo::BeginFrame();
			ImGuizmo::SetOrthographic(false);
			/* ---- ImGuizmo ---- */

			// Create the docking environment
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
				ImGuiWindowFlags_NoBackground;

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::Begin("InvisibleWindow", nullptr, windowFlags);
			ImGui::PopStyleVar(3);

			ImGuiID dockSpaceId = ImGui::GetID("InvisibleWindowDockSpace");
			ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

			ImGui::End();
			// ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport()); // Removed duplicate dockspace

			ShowMenuBar();
		}

		void ImGuiSystem::ShowMenuBar()
		{
			// Create a menu bar
			if (ImGui::BeginMainMenuBar())
			{
				// File menu
				if (ImGui::BeginMenu("File"))
				{
					ImGui::BeginDisabled(PrefabEditModeController::GetInstance().isActive());

					if (ImGui::MenuItem("New", "Ctrl+N")) 
					{
						SceneTools::GetInstance().Enable(SceneTools::NEW);
					}
					if (ImGui::MenuItem("Open...", "Ctrl+O"))
					{
						SceneTools::GetInstance().Enable(SceneTools::OPEN);
					}
					if (ImGui::MenuItem("Save", "Ctrl+S")) {
						// Open the SceneTools Save dialog (lets user choose Save vs Save As)
						SceneTools::GetInstance().Enable(SceneTools::SAVE);
					}
					if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) 
					{
						SceneTools::GetInstance().Enable(SceneTools::SAVEAS);
					}

					if (ImGui::MenuItem("Export...", "Ctrl+E")) 
					{
						m_showExportSettings = true;
					}

					ImGui::EndDisabled();

					ImGui::Separator(); // Adds a dividing line
					if (ImGui::MenuItem("Exit", "Alt+F4")) { glfwSetWindowShouldClose(AGWINDOW.getWindowContext(), 1); }

					ImGui::EndMenu();
				}

				// Edit menu
				if (ImGui::BeginMenu("Edit"))
				{
					if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* Undo action */ }
					if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) { /* Redo action (disabled) */ }
					ImGui::Separator();
					if (ImGui::MenuItem("Cut", "Ctrl+X")) { /* Cut action */ }
					if (ImGui::MenuItem("Copy", "Ctrl+C")) { /* Copy action */ }
					if (ImGui::MenuItem("Paste", "Ctrl+V")) { /* Paste action */ }
					ImGui::EndMenu();
				}

				// Help menu
				if (ImGui::BeginMenu("Help"))
				{
					if (ImGui::MenuItem("About")) { ImGui::OpenPopup("AboutPage"); }
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Tool"))
				{
					if (ImGui::MenuItem("Asset Compiler")) { AssetCompiler::GetInstance().Enable(); }
					if (ImGui::MenuItem("Prefab Editor")) { PrefabEditor::GetInstance().Enable(); }
					if (ImGui::MenuItem("BHT Editor")) { AG::AGImGui::BHTEditor::Enable(); }
					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}
			if (m_showExportSettings) ShowExportSettings();
		}

		void ImGuiSystem::ShowExportSettings()
		{
			ImGui::Begin("Export Settings", &m_showExportSettings, ImGuiWindowFlags_AlwaysAutoResize);

			static std::string m_appName, m_openingScene;
			static float m_appWindowSize[2] = {0.f, 0.f};
			float width = ImGui::GetContentRegionAvail().x;	

			ImGui::Text("Application Name:"); ImGui::SameLine(); 
			ImGui::CalcTextSize("Application Name:");
			ImGui::SetCursorPosX(ImGui::CalcTextSize("Application Name:").x + 5.f);
			ImGui::SetNextItemWidth(width - ImGui::CalcTextSize("Application Name:").x - 0.5f);
			ImGui::InputText("##AppName", &m_appName);

			ImGui::Text("Application Window Size:"); ImGui::SameLine();
			ImGui::InputFloat2("##AppWindowSize", m_appWindowSize, "%.0f");
			
			ImGui::Text("Opening Scene:"); ImGui::SameLine();
			ImGui::CalcTextSize("Opening Scene:");
			ImGui::SetCursorPosX(ImGui::CalcTextSize("Opening Scene:").x + 5.f);
			ImGui::SetNextItemWidth(width - ImGui::CalcTextSize("Opening Scene:").x - 0.5f);
			ImGui::InputText("##OpeningScene", &m_openingScene);

			if (ImGui::Button("Export"))
			{
				rapidjson::Document document;
				document.SetObject();
				rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
				rapidjson::Value appNameValue(m_appName.c_str(), allocator);
				document.AddMember("AppName", appNameValue, allocator);

				document.AddMember("WindowWidth", (int)m_appWindowSize[0], allocator);

				document.AddMember("WindowHeight", (int)m_appWindowSize[1], allocator);

				rapidjson::Value sceneValue(m_openingScene.c_str(), allocator);
				document.AddMember("OpeningScene", sceneValue, allocator);

				rapidjson::StringBuffer buffer;

				rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

				document.Accept(writer);

				std::ofstream e_ofs("../Export/GameConfig.json");
				if (e_ofs.is_open())
				{
					e_ofs << buffer.GetString();
					e_ofs.close();
					AG_CORE_INFO("[EXPORT] Exported GameConfig.json successfully.");
				}

				std::ofstream ofs("GameConfig.json");
				if (ofs.is_open())
				{
					ofs << buffer.GetString();
					ofs.close();
					AG_CORE_INFO("[EXPORT] Exported GameConfig.json successfully.");
				}
				m_showExportSettings = false;
			}

			ImGui::End();
		}

		void ImGuiSystem::Render() {
			for (size_t i{ 0 }; i < m_windows.size(); ++i) {
				m_windows[i]->Render();
			}
		}

		void ImGuiSystem::PostRender() {
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			ImGuiIO& io = ImGui::GetIO();

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
		}

		void ImGuiSystem::AddWindow(ImguiWindowPtr component) {
			m_windows.push_back(std::move(component));
		}

		void ImGuiSystem::RipeIGuiStyle()
		{
			ImGuiStyle& style = ImGui::GetStyle();
			ImVec4* colors = style.Colors;

			// Backgrounds
			colors[ImGuiCol_WindowBg] = ImVec4(1.0f, 0.937f, 0.655f, 1.00f);   // Soft yellow for window background
			colors[ImGuiCol_ChildBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);    // Slightly lighter yellow
			colors[ImGuiCol_PopupBg] = ImVec4(0.98f, 0.92f, 0.46f, 1.00f);    // Light yellow for popup background

			// Text
			colors[ImGuiCol_Text] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);      // Dark brownish-black text for good contrast
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

			// Borders
			colors[ImGuiCol_Border] = ImVec4(0.58f, 0.45f, 0.18f, 1.00f);    // Brownish border for contrast
			colors[ImGuiCol_BorderShadow] = ImVec4(0.50f, 0.40f, 0.20f, 0.50f);

			// Headers
			colors[ImGuiCol_Header] = ImVec4(0.88f, 0.76f, 0.16f, 1.00f);    // Brighter yellow for headers
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.92f, 0.78f, 0.22f, 1.00f); // Slightly darker on hover
			colors[ImGuiCol_HeaderActive] = ImVec4(0.93f, 0.81f, 0.25f, 1.00f);  // Active header in banana-like yellow

			// Buttons
			colors[ImGuiCol_Button] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);   // Light gray button
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f); // Slightly darker gray when hovered
			colors[ImGuiCol_ButtonActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);  // Darker gray when active

			// Frames
			colors[ImGuiCol_FrameBg] = ImVec4(0.95f, 0.86f, 0.36f, 1.00f);  // Frame background in light yellow
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.97f, 0.89f, 0.44f, 1.00f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.92f, 0.80f, 0.30f, 1.00f);

			// Title
			colors[ImGuiCol_TitleBg] = ImVec4(0.91f, 0.729f, 0.008f, 1.00f);  // Yellowish title background
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.95f, 0.88f, 0.43f, 1.00f); // Lighter yellow for collapsed
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.87f, 0.76f, 0.18f, 1.00f); // Darker yellow for active title

			// Scrollbar
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.93f, 0.85f, 0.35f, 1.00f);  // Light yellow
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.87f, 0.76f, 0.16f, 1.00f);  // Banana peel yellow
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.89f, 0.79f, 0.20f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.92f, 0.81f, 0.25f, 1.00f);

			// Checkmark
			colors[ImGuiCol_CheckMark] = ImVec4(0.58f, 0.48f, 0.18f, 1.00f);   // Brownish checkmark

			// Slider
			colors[ImGuiCol_SliderGrab] = ImVec4(0.87f, 0.76f, 0.16f, 1.00f);    // Banana yellow for slider
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.91f, 0.79f, 0.20f, 1.00f);  // Darker yellow on grab

			// Tabs
			colors[ImGuiCol_Tab] = ImVec4(0.95f, 0.85f, 0.36f, 1.00f);      // Tab in banana yellow
			colors[ImGuiCol_TabHovered] = ImVec4(0.97f, 0.87f, 0.42f, 1.00f);
			colors[ImGuiCol_TabActive] = ImVec4(0.831f, 0.706f, 0.149f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.91f, 0.82f, 0.36f, 1.00f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.88f, 0.78f, 0.25f, 1.00f);

			// Plot lines
			colors[ImGuiCol_PlotLines] = ImVec4(0.60f, 0.50f, 0.18f, 1.00f);    // Light brown lines
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.70f, 0.60f, 0.28f, 1.00f);

			// Resize grip
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.90f, 0.80f, 0.35f, 1.00f);   // Banana peel yellow for resizing
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.95f, 0.86f, 0.42f, 1.00f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.89f, 0.44f, 1.00f);
		}

		void ImGuiSystem::UnityLightStyle()
		{
			ImGuiStyle& style = ImGui::GetStyle();
			ImVec4* colors = style.Colors;

			// === Text ===
			colors[ImGuiCol_Text] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // black
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // gray

			// === Backgrounds ===
			colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f); // main window
			colors[ImGuiCol_ChildBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);

			// === Borders ===
			colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

			// === Frames ===
			colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);

			// === Title bar ===
			colors[ImGuiCol_TitleBg] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.95f, 0.95f, 0.95f, 0.75f);

			// === Menu bar ===
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);

			// === Scrollbars ===
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.60f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.60f, 0.60f, 0.60f, 0.80f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

			// === Checkboxes, radios ===
			colors[ImGuiCol_CheckMark] = ImVec4(0.25f, 0.52f, 0.95f, 1.00f); // blue check

			// === Sliders ===
			colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

			// === Buttons ===
			colors[ImGuiCol_Button] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

			// === Headers (Tree nodes, collapsing headers, etc) ===
			colors[ImGuiCol_Header] = ImVec4(0.75f, 0.80f, 0.90f, 0.45f); // selection
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.60f, 0.70f, 0.90f, 0.80f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.50f, 0.65f, 0.85f, 1.00f);

			// === Separators ===
			colors[ImGuiCol_Separator] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

			// === Resize grip ===
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

			// === Tabs ===
			colors[ImGuiCol_Tab] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.70f, 0.78f, 0.90f, 0.80f);
			colors[ImGuiCol_TabActive] = ImVec4(0.65f, 0.75f, 0.85f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.75f, 0.82f, 0.88f, 1.00f);

			// === Plots ===
			colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.80f, 0.60f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

			// === Text selection & drag/drop ===
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

			// === Navigation / modal dim ===
			colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

			style.WindowRounding = 4.0f;
			style.FrameRounding = 3.0f;
			style.GrabRounding = 2.0f;
			style.ScrollbarSize = 13.0f;
			style.WindowBorderSize = 1.0f;
			style.FrameBorderSize = 0.0f;
			style.ItemSpacing = ImVec2(8, 4);
			style.ItemInnerSpacing = ImVec2(6, 4);

			style.TabRounding = 8.0f;
			style.FrameRounding = 8.0f;
		}

		void ImGuiSystem::UnityDarkStyle() {
			ImGuiStyle& style = ImGui::GetStyle();
			ImVec4* colors = style.Colors;

			// === Text ===
			colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f); // default text
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // grayed-out/inactive text

			// === Window & Backgrounds ===
			colors[ImGuiCol_WindowBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f); // main window background
			colors[ImGuiCol_ChildBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f); // child windows (e.g. panels)
			colors[ImGuiCol_PopupBg] = ImVec4(0.22f, 0.22f, 0.22f, 0.94f); // popup windows and combo boxes

			// === Borders & Separators ===
			colors[ImGuiCol_Border] = ImVec4(0.40f, 0.40f, 0.40f, 0.30f); // outer edge of windows
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // unused

			// === Frames (e.g. input fields, dropdowns) ===
			colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // normal background
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f); // when hovered
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f); // when clicked/active

			// === Title Bar ===
			colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // normal
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); // active window
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 0.75f); // collapsed state

			// === Menu Bar ===
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); // top menu bar

			// === Scrollbars ===
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // track
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.40f, 0.60f); // handle
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.80f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

			// === Checkboxes, Radio Buttons ===
			colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // the check itself

			// === Sliders, Grabs ===
			colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

			// === Buttons ===
			colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // normal
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);

			// === Headers (TreeNodes, Selectables, CollapsingHeaders) ===
			colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f); // selected background
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f); // hover
			colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // active

			// === Separators ===
			colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.75f, 0.75f, 0.78f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);

			// === Resize Grip ===
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

			// === Tabs ===
			colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f); // inactive tab
			colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f); // hovered tab
			colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f); // selected tab
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f); // background tabs
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f); // background selected tab

			// === Plots ===
			colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

			// === Text Selection Highlight ===
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f); // selected text background

			// === Drag-and-Drop ===
			colors[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // yellow target glow

			// === Navigation & Windowing ===
			colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);


			style.WindowRounding = 3.0f;
			style.FrameRounding = 3.0f;
			style.GrabRounding = 2.0f;
			style.ScrollbarSize = 14.0f;
			style.WindowBorderSize = 1.0f;
			style.FrameBorderSize = 0.0f;
			style.ItemSpacing = ImVec2(8, 4);
			style.ItemInnerSpacing = ImVec2(6, 4);

			style.TabRounding = 4.0f;
			style.FrameRounding = 4.0f;
		}

		void ImGuiSystem::VSStudioStyle()
		{
			// Visual Studio style by MomoDeve from ImThemes
			ImGuiStyle& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.DisabledAlpha = 0.6000000238418579f;
			style.WindowPadding = ImVec2(8.0f, 8.0f);
			style.WindowRounding = 0.0f;
			style.WindowBorderSize = 1.0f;
			style.WindowMinSize = ImVec2(32.0f, 32.0f);
			style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_Left;
			style.ChildRounding = 0.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 0.0f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(4.0f, 3.0f);
			style.FrameRounding = 0.0f;
			style.FrameBorderSize = 0.0f;
			style.ItemSpacing = ImVec2(8.0f, 4.0f);
			style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
			style.CellPadding = ImVec2(4.0f, 2.0f);
			style.IndentSpacing = 21.0f;
			style.ColumnsMinSpacing = 6.0f;
			style.ScrollbarSize = 14.0f;
			style.ScrollbarRounding = 0.0f;
			style.GrabMinSize = 10.0f;
			style.GrabRounding = 0.0f;
			style.TabRounding = 0.0f;
			style.TabBorderSize = 0.0f;
			style.TabMinWidthForCloseButton = 0.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5921568870544434f, 0.5921568870544434f, 0.5921568870544434f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, 1.0f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
			style.Colors[ImGuiCol_Separator] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, 1.0f);
			style.Colors[ImGuiCol_Tab] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
		}

		void ImGuiSystem::SetSelectedObj(const std::shared_ptr<System::IObject>& object) 
		{ 
			m_selectedObject = object; 
			PrefabEditModeController::GetInstance().setNewObjID(object); //this is to check for prefab edit mode
		}
	}
}