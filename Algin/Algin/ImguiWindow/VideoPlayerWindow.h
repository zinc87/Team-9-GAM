/*!*****************************************************************************
\file VideoPlayerWindow.h
\brief ImGui window for playing videos inside the editor.
*******************************************************************************/
#pragma once
#include "IImguiWindow.h"
#include "../Graphics/VideoTexture.h"

namespace AG {
	namespace AGImGui {

		class VideoPlayerWindow : public IImguiWindow
		{
		public:
			VideoPlayerWindow();
			virtual ~VideoPlayerWindow();

			void Render() override;

		private:
			// Video Playback
			VideoTexture m_video;
			bool m_isPlaying = false;
			float m_currentTime = 0.0f; // Tracked mostly for UI slider
			float m_duration = 0.0f;    // Estimated duration if available

			// FBO for off-screen rendering
			GLuint m_fbo = 0;
			GLuint m_fboTexture = 0;
			int m_fboWidth = 0;
			int m_fboHeight = 0;

			// Quad Rendering Resources (copied from simple quad logic)
			GLuint m_quadVAO = 0;
			GLuint m_quadVBO = 0;

			void InitFBO(int width, int height);
			void ResizeFBO(int width, int height);
			void DestroyFBO();

			void CreateQuad();
			void DrawQuad();

			// Handles the YUV->RGB render pass
			void RenderVideoToFBO();
		};
	}
}
