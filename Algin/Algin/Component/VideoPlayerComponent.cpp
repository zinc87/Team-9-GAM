/*!*****************************************************************************
\file VideoPlayerComponent.cpp
\brief Implementation of runtime Video Player component.
*******************************************************************************/
#include "pch.h"
#include "VideoPlayerComponent.h"
#include "../Graphics/Shaders.h" 
#include "../Graphics/Camera.h"
#include "../Data/Reflection.h"
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>

namespace fs = std::filesystem;

// Register VideoPlayerComponent with the TypeRegistry for serialization
REGISTER_REFLECTED_TYPE(AG::Component::VideoPlayerComponent)

namespace AG {
    namespace Component {

        // Helper to scan for video files
        static std::vector<std::string> GetVideoAssets(const std::string& directory) {
            std::vector<std::string> videos;
            if (fs::exists(directory) && fs::is_directory(directory)) {
                for (const auto& entry : fs::directory_iterator(directory)) {
                    if (entry.is_regular_file()) {
                        std::string ext = entry.path().extension().string();
                        // Case insensitive check would be better, but strict check for now
                        if (ext == ".mpg" || ext == ".mpeg" || ext == ".mp4" || ext == ".avi") {
                            videos.push_back(entry.path().filename().string());
                        }
                    }
                }
            }
            return videos;
        }

        VideoPlayerComponent::VideoPlayerComponent() 
            : IComponent(Data::ComponentTypes::VideoPlayer)
        {
        }

        VideoPlayerComponent::VideoPlayerComponent(ID id, ID objId)
            : IComponent(Data::ComponentTypes::VideoPlayer, id, objId)
        {
        }

        VideoPlayerComponent::~VideoPlayerComponent()
        {
            Free();
        }

        void VideoPlayerComponent::Awake() { }

        void VideoPlayerComponent::Start()
        {
            if (m_playOnStart && !m_videoPath.empty()) {
                Play(m_videoPath);
            }
        }

        void VideoPlayerComponent::Update()
        {
            if (m_videoTexture.IsLoaded())
            {
                // Update video decoding
                float dt = static_cast<float>(BENCHMARKER.GetDeltaTime());
                m_videoTexture.Update(dt);

                // Render to FBO or Screen
                // In a component context, we usually don't invoke draw commands directly in Update()
                // unless we are bypassing the renderer. 
                // For a Dedicated Scene, this is acceptable if we render a fullscreen quad.
            }
        }

        void VideoPlayerComponent::LateUpdate()
        {
            // Only render during the Game Camera pass (not Scene or Canvas)
            // This ensures video draws to hdrFBO which feeds into the Game View
            auto currentCam = CAMERAMANAGER.getCurrentCamera().lock();
            auto gameCam = CAMERAMANAGER.getGameCamera().lock();
            if (!currentCam || !gameCam || currentCam != gameCam)
                return;

            // Draw here to overlay on top of everything
            if (m_videoTexture.IsLoaded())
            {
                RenderFullscreen();
            }
        }

        void VideoPlayerComponent::Free()
        {
            m_videoTexture.Unload();
            if (m_fbo) { glDeleteFramebuffers(1, &m_fbo); m_fbo = 0; }
            if (m_fboTexture) { glDeleteTextures(1, &m_fboTexture); m_fboTexture = 0; }
            if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
        }

        void VideoPlayerComponent::Inspector()
        {
            ImGui::Text("Video Player Settings");
            
            // Asset Picker Logic
            // 1. Scan Assets/Videos
            static std::vector<std::string> videoFiles;
            // Re-scan occasionally or if empty. For now, scan every time to be responsive to new files.
            // Ideally should be event-driven or cached.
            videoFiles = GetVideoAssets("Assets/Videos");

            // 2. Search Filter
            static char searchBuffer[128] = "";
            ImGui::InputTextWithHint("Search", "Filter videos...", searchBuffer, sizeof(searchBuffer));
            std::string searchStr = searchBuffer;
            std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            // 3. Dropdown
            // Show current selection or "None"
            std::string previewValue = m_videoPath.empty() ? "None" : std::filesystem::path(m_videoPath).filename().string();
            
            if (ImGui::BeginCombo("Video", previewValue.c_str()))
            {
                for (const auto& file : videoFiles)
                {
                    // Filter Logic
                    std::string fileLower = file;
                    std::transform(fileLower.begin(), fileLower.end(), fileLower.begin(),
                        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                    if (!searchStr.empty() && fileLower.find(searchStr) == std::string::npos)
                        continue;

                    bool isSelected = (previewValue == file);
                    if (ImGui::Selectable(file.c_str(), isSelected))
                    {
                        m_videoPath = "Assets/Videos/" + file;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::TextWrapped("Path: %s", m_videoPath.c_str());

            // Drag Drop Support
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("VIDEO_FILE")) {
                    m_videoPath = (const char*)payload->Data;
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::Separator();
            ImGui::Checkbox("Play On Start", &m_playOnStart);
            ImGui::Checkbox("Loop", &m_looping);
            m_videoTexture.SetLooping(m_looping);

            if (ImGui::Button("Play")) Play(m_videoPath);
            ImGui::SameLine();
            if (ImGui::Button("Stop")) Stop();

            if (m_videoTexture.IsLoaded()) {
                ImGui::Text("Playing: %s (%dx%d)", m_videoPath.c_str(), m_videoTexture.GetWidth(), m_videoTexture.GetHeight());
            }
        }

        void VideoPlayerComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp)
        {
            auto source = std::dynamic_pointer_cast<VideoPlayerComponent>(fromCmp);
            if (source) {
                m_videoPath = source->m_videoPath;
                m_playOnStart = source->m_playOnStart;
                m_looping = source->m_looping;
            }
        }

        bool VideoPlayerComponent::Play(const std::string& path)
        {
            m_videoPath = path; // Sync path
            if (m_videoTexture.Load(m_videoPath)) {
                m_videoTexture.SetLooping(m_looping);
                InitRenderResources();
                return true;
            }
            return false;
        }

        void VideoPlayerComponent::Stop()
        {
            m_videoTexture.Unload();
        }

        bool VideoPlayerComponent::IsFinished() const
        {
            return m_videoTexture.HasEnded();
        }

        void VideoPlayerComponent::InitRenderResources()
        {
            if (m_vao == 0) glGenVertexArrays(1, &m_vao); // Empty VAO for full-screen shader
            
            // FBO Init logic (Similar to Editor Window if we need to post-process)
            // For simple fullscreen, we can just render to Backbuffer directly using shader.
            // But let's stick to simple "Draw Quad" logic.
        }

        void VideoPlayerComponent::RenderFullscreen()
        {
            // Use the VideoYUV shader
            // Note: SHADERMANAGER must have loaded "VideoYUV"
            
            ShaderManager::GetInstance().Use("VideoYUV");
            GLuint program = ShaderManager::GetInstance().GetShaderProgram(); 

            // Bind Textures
            m_videoTexture.BindYUV();

            // Notify shader of texture units
            glUniform1i(glGetUniformLocation(program, "uTexY"), 0);
            glUniform1i(glGetUniformLocation(program, "uTexCb"), 1);
            glUniform1i(glGetUniformLocation(program, "uTexCr"), 2);

            // Disable depth test to ensure video draws over everything
            GLboolean depthEnabled;
            glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);

            // Draw Fullscreen Quad
            glBindVertexArray(m_vao);
            glDrawArrays(GL_TRIANGLES, 0, 3); // 3 vertices for full screen triangle
            glBindVertexArray(0);
            
            // Restore depth state
            glDepthMask(GL_TRUE);
            if (depthEnabled) glEnable(GL_DEPTH_TEST);

            glUseProgram(0);
        }
    }
}
