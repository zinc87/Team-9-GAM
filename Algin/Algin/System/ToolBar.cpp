/*!*****************************************************************************
\file ToolBar.cpp
\author Zulfami Ashrafi Bin Wakif
\date 20/1/2025 (MM/DD/YYYY)

\brief Tool Bar for the engine
*******************************************************************************/
#pragma once
#include "pch.h"

namespace AG {
    namespace AGImGui {
        ToolBarImgui::~ToolBarImgui() {

        }

        void ToolBarImgui::Render() {
            ImGui::SetNextWindowSize(ImVec2(m_size.first, m_size.second));
            if (ImGui::Begin(m_name.c_str())) {
                const int ButtonWidth = 100;
                const int ButtonHeightOffset = 15;
                const int ButtonCount = 3;

                // Get window size and cursor pos
                ImVec2 windowSize = ImGui::GetWindowSize();
                ImVec2 buttonSize(ButtonWidth, windowSize.y - ButtonHeightOffset); // width, height of each button

                float totalButtonsWidth = (buttonSize.x * ButtonCount) + (ImGui::GetStyle().ItemSpacing.x * ButtonCount - 1);
                float startX = (windowSize.x - totalButtonsWidth) * 0.5f;
                float startY = (windowSize.y - buttonSize.y) * 0.5f;

                ImGui::Text(("Current State: " + SCENESTATEMANAGER.GetSceneStateStr()).c_str());

                ImGui::SetCursorPos(ImVec2(startX, startY));

                // Hot Key to stop
                if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_F7)) {
                    SCENESTATEMANAGER.SetSceneState(System::SceneManager::STOPPED);
                    SCENESTATEMANAGER.RunOnEnd();
                    glfwSetInputMode(AGWINDOW.getWindowContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }

                // Hot Key to start
                if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_F5)) {
                    SCENESTATEMANAGER.SetSceneState(System::SceneManager::PLAYING);
                    SCENESTATEMANAGER.RunOnStart();
                }

                // Hot Key to pause
                if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_F6)) {
                    SCENESTATEMANAGER.SetSceneState(System::SceneManager::PAUSED);
                    SCENESTATEMANAGER.RunOnPause();
                }

                if (ImGui::Button("Play", buttonSize)) {
                    SCENESTATEMANAGER.SetSceneState(System::SceneManager::PLAYING);
                    SCENESTATEMANAGER.RunOnStart();
                }
                ImGui::SameLine();
                if (ImGui::Button("Pause", buttonSize)) {
                    SCENESTATEMANAGER.SetSceneState(System::SceneManager::PAUSED);
                    SCENESTATEMANAGER.RunOnPause();
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop", buttonSize)) {
                    SCENESTATEMANAGER.SetSceneState(System::SceneManager::STOPPED);
                    SCENESTATEMANAGER.RunOnEnd();
                    glfwSetInputMode(AGWINDOW.getWindowContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                }
            }
            ImGui::End();
        }
    }


    namespace System {

    }
}