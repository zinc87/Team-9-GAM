/*!*****************************************************************************
\file VideoPlayerComponent.h
\brief Runtime component for playing videos in a dedicated scene context.
*******************************************************************************/
#pragma once
#include "pch.h"
#include "../Graphics/VideoTexture.h"

namespace AG {
    namespace Component {

        class VideoPlayerComponent : public IComponent {
        public:
            using Self = VideoPlayerComponent;
            static Type GetStaticType() { return Data::ComponentTypes::VideoPlayer; }

            VideoPlayerComponent();
            VideoPlayerComponent(ID id, ID objId);
            virtual ~VideoPlayerComponent();

            // Inherited via IComponent
            void Awake() override;
            void Start() override;
            void Update() override;
            void LateUpdate() override;
            void Free() override;
            void Inspector() override;
            void AssignFrom(const std::shared_ptr<IComponent>& fromCmp) override;

            std::string GetTypeName() const override { return "AG::Component::VideoPlayerComponent"; }

            // -- Runtime Interface --
            bool Play(const std::string& path);
            void Stop();
            bool IsFinished() const;

            // -- Rendering --
            // Draws full-screen quad using m_videoTexture
            void RenderFullscreen(); 

            REFLECT() {
                return {
                    REFLECT_FIELD(m_videoPath, std::string, "Video Path"),
                    REFLECT_FIELD(m_playOnStart, bool, "Play On Start"),
                    REFLECT_FIELD(m_looping, bool, "Loop")
                };
            }

            REFLECT_SERIALIZABLE(VideoPlayerComponent)

        private:
            std::string m_videoPath;
            bool m_playOnStart = true;
            bool m_looping = false;

            VideoTexture m_videoTexture;
            
            // Rendering resources
            GLuint m_fbo = 0;
            GLuint m_fboTexture = 0;
            int m_fboWidth = 0;
            int m_fboHeight = 0;
            GLuint m_shader = 0; 
            GLuint m_vao = 0; // Empty VAO for full-screen quad

            void InitRenderResources();
            void DrawVideoToFBO();
        };
    }
}
