#include "pch.h"
#include "VideoPlayerWindow.h"

namespace AG {
	namespace AGImGui {

		VideoPlayerWindow::VideoPlayerWindow()
			: IImguiWindow({ 800.f, 600.f }, "Video Player")
		{
		}

		VideoPlayerWindow::~VideoPlayerWindow()
		{
			DestroyFBO();
			if (m_quadVAO != 0) {
				glDeleteVertexArrays(1, &m_quadVAO);
				glDeleteBuffers(1, &m_quadVBO);
			}
			m_video.Unload();
		}

		void VideoPlayerWindow::Render()
		{
			ImGui::Begin("Video Player", nullptr); // Optional bool* open

			// 1. Drag & Drop Target
			// We can make the whole window a target, or a specific child.
			// Let's make the view area a target.

			// Check if we need to resize FBO
			int videoW = m_video.GetWidth();
			int videoH = m_video.GetHeight();

			// If video is loaded but FBO not matching, resize (or create)
			if (m_video.IsLoaded())
			{
				if (m_fboWidth != videoW || m_fboHeight != videoH)
				{
					ResizeFBO(videoW, videoH);
				}
			}

			// Update Video
			if (m_isPlaying && m_video.IsLoaded())
			{
				float dt = ImGui::GetIO().DeltaTime;
				m_video.Update(dt);
				
				if (m_video.HasEnded())
				{
					if (m_video.IsLooping())
						m_video.Rewind();
					else
						m_isPlaying = false;
				}
			}

			// Render to FBO
			if (m_video.IsLoaded() && m_fbo != 0)
			{
				RenderVideoToFBO();
			}

			// Display in ImGui
			float availW = ImGui::GetContentRegionAvail().x;
			float availH = ImGui::GetContentRegionAvail().y - 40.0f; // Leave space for controls
			
			// Aspect ratio handling
			float aspect = (videoH > 0) ? (float)videoW / (float)videoH : 16.0f / 9.0f;
			float viewH = availW / aspect;
			if (viewH > availH) {
				viewH = availH;
				availW = viewH * aspect;
			}
			
			ImVec2 imageSize = ImVec2(availW, viewH);
			ImTextureID texID = (ImTextureID)(uintptr_t)m_fboTexture;

			// If no video, show placeholder or create placeholder FBO?
			// Just show a dummy box or text if not loaded
			if (m_video.IsLoaded())
			{
				// OpenGL texture is uploaded bottom-to-top relative to buffer? 
				// Actually pl_mpeg is top-to-bottom. GL is bottom-to-top.
				// So data is effectively "upside down" in memory = "right side up" for UV sampling (V=0 is top).
				// We use standard UVs (0,0 to 1,1).
				ImGui::Image(texID, imageSize, ImVec2(0, 1), ImVec2(1, 0));

			}
			else
			{
				ImGui::BeginChild("VideoPlaceholder", imageSize, true);
				ImVec2 center = ImVec2(imageSize.x * 0.5f, imageSize.y * 0.5f);
				ImGui::SetCursorPos(ImVec2(center.x - 60, center.y - 10));
				ImGui::Text("Drag .mpg file here");
				ImGui::EndChild();
			}

			// DRAG DROP TARGET
			// Use LastItem (Image or Child) as target
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("VIDEO_FILE"))
				{
					const char* path = (const char*)payload->Data;
					AG_CORE_INFO("[VideoPlayerWindow] Dropped file: {}", path);
					
					// Load the video
					if (m_video.Load(path))
					{
						m_isPlaying = true; // Auto play
						m_video.SetLooping(true);
					}
				}
				ImGui::EndDragDropTarget();
			}

			// Controls
			ImGui::Separator();
			if (ImGui::Button(m_isPlaying ? "Pause" : "Play"))
			{
				m_isPlaying = !m_isPlaying;
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop"))
			{
				m_isPlaying = false;
				m_video.Rewind();
			}
			ImGui::SameLine();
			// Loop Toggle
			bool loop = m_video.IsLooping();
			if (ImGui::Checkbox("Loop", &loop))
			{
				m_video.SetLooping(loop);
			}

			ImGui::End();
		}

		void VideoPlayerWindow::InitFBO(int width, int height)
		{
			if (width <= 0 || height <= 0) return;

			glGenFramebuffers(1, &m_fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

			glGenTextures(1, &m_fboTexture);
			glBindTexture(GL_TEXTURE_2D, m_fboTexture);
			// RGB8 is enough
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
			// Attach to FBO
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTexture, 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				AG_CORE_ERROR("[VideoPlayerWindow] FBO Incomplete!");
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			m_fboWidth = width;
			m_fboHeight = height;
		}

		void VideoPlayerWindow::ResizeFBO(int width, int height)
		{
			DestroyFBO();
			InitFBO(width, height);
		}

		void VideoPlayerWindow::DestroyFBO()
		{
			if (m_fboTexture != 0) {
				glDeleteTextures(1, &m_fboTexture);
				m_fboTexture = 0;
			}
			if (m_fbo != 0) {
				glDeleteFramebuffers(1, &m_fbo);
				m_fbo = 0;
			}
			m_fboWidth = 0;
			m_fboHeight = 0;
		}

		void VideoPlayerWindow::CreateQuad()
		{
			if (m_quadVAO != 0) return;

			float quadVertices[] = {
				// positions   // texcoords
				-1.0f,  1.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f,
				 1.0f, -1.0f, 1.0f, 0.0f,

				-1.0f,  1.0f, 0.0f, 1.0f,
				 1.0f, -1.0f, 1.0f, 0.0f,
				 1.0f,  1.0f, 1.0f, 1.0f
			};

			glGenVertexArrays(1, &m_quadVAO);
			glGenBuffers(1, &m_quadVBO);
			glBindVertexArray(m_quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			glBindVertexArray(0);
		}

		void VideoPlayerWindow::DrawQuad()
		{
			CreateQuad();
			glBindVertexArray(m_quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}

		void VideoPlayerWindow::RenderVideoToFBO()
		{
			// Save previous state
			GLint prevFBO;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
			GLint prevViewport[4];
			glGetIntegerv(GL_VIEWPORT, prevViewport);

			// Bind our FBO
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
			glViewport(0, 0, m_fboWidth, m_fboHeight);
			//glClear(GL_COLOR_BUFFER_BIT); // Not strictly needed if we draw full quad, but good practice

			// Use Video Shader
			// Assumes ShaderManager has "VideoYUV" loaded.
			SHADERMANAGER.Use("VideoYUV");
			
			// Bind YUV textures
			m_video.BindYUV();

			// Draw Quad
			DrawQuad();

			// Restore
			glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
			glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
		}
	}
}
