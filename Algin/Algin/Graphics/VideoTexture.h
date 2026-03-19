/*!*****************************************************************************
\file VideoTexture.h
\brief Runtime class for MPEG-1 video playback using pl_mpeg.
       Decodes video frames and uploads YUV planes to GPU textures for
       shader-based YUV→RGB conversion. This avoids CPU bottlenecks
       and matches pl_mpeg's intended usage pattern.
*******************************************************************************/
#pragma once

// Forward declare pl_mpeg types to avoid including heavy header in pch
// pl_mpeg uses anonymous struct typedefs, so we must include the header
// instead of forward declaring types.
// VideoTexture.h includes pl_mpeg.h, which provides full type definitions
// pl_mpeg uses anonymous struct typedefs, so we must include the header
// instead of forward declaring types.
#include <stdio.h>
#include "../../extern/pl_mpeg/pl_mpeg.h"
#include <fmod.hpp>
#include <fmod_errors.h> 
#include <mutex>
#include <vector>

// Forward declarations for other types if needed
// struct plm_t; // Defined in pl_mpeg.h
// struct plm_frame_t; // Defined as anonymous typedef in pl_mpeg.h

/*!*****************************************************************************
\class VideoTexture
\brief Manages MPEG-1 video decoding and GPU texture upload for video cutscenes.

Usage:
    VideoTexture video;
    video.Load("Assets/Videos/intro.mpg");

    // In update loop:
    video.Update(deltaTime);
    if (!video.HasEnded()) {
        SHADERMANAGER.Use("VideoYUV");
        video.BindYUV();
        // Draw fullscreen quad
    }
*******************************************************************************/
class VideoTexture
{
public:
    VideoTexture();
    ~VideoTexture();

    // Prevent copying - we own GPU resources
    VideoTexture(const VideoTexture&) = delete;
    VideoTexture& operator=(const VideoTexture&) = delete;

    // Allow moving
    VideoTexture(VideoTexture&& other) noexcept;
    VideoTexture& operator=(VideoTexture&& other) noexcept;

    /*!*************************************************************************
    \brief Load an MPEG-1 video file (.mpg) for playback.
    \param path File path to the .mpg file
    \return true if loaded successfully, false otherwise
    ***************************************************************************/
    bool Load(const std::string& path);

    /*!*************************************************************************
    \brief Release all resources (decoder, textures, buffers).
    ***************************************************************************/
    void Unload();

    /*!*************************************************************************
    \brief Advance video playback by the given delta time.
           Internally calls plm_decode() to decode the next frame(s).
    \param dt Delta time in seconds since last update
    ***************************************************************************/
    void Update(float dt);

    /*!*************************************************************************
    \brief Bind the Y, Cb, Cr textures to texture units 0, 1, 2.
           Call this before drawing a fullscreen quad with the VideoYUV shader.
    ***************************************************************************/
    void BindYUV();

    /*!*************************************************************************
    \brief Check if video playback has ended.
    \return true if video has finished playing
    ***************************************************************************/
    bool HasEnded() const { return m_hasEnded; }

    /*!*************************************************************************
    \brief Check if a video is currently loaded.
    \return true if decoder is active and ready
    ***************************************************************************/
    bool IsLoaded() const { return m_decoder != nullptr; }

    /*!*************************************************************************
    \brief Get the video frame width in pixels.
    \return Width in pixels, or 0 if not loaded
    ***************************************************************************/
    int GetWidth() const { return m_width; }

    /*!*************************************************************************
    \brief Get the video frame height in pixels.
    \return Height in pixels, or 0 if not loaded
    ***************************************************************************/
    int GetHeight() const { return m_height; }

    /*!*************************************************************************
    \brief Reset playback to the beginning of the video.
           Useful for looping playback.
    ***************************************************************************/
    void Rewind();

    /*!*************************************************************************
    \brief Set whether the video should loop automatically.
    \param loop true to enable looping, false for one-shot playback
    ***************************************************************************/
    void SetLooping(bool loop) { m_looping = loop; }

    /*!*************************************************************************
    \brief Check if looping is enabled.
    \return true if video will loop when finished
    ***************************************************************************/
    bool IsLooping() const { return m_looping; }

private:
    // pl_mpeg decoder handle
    plm_t* m_decoder = nullptr;

    // OpenGL texture handles for YUV planes
    // Each plane is stored as a single-channel R8 texture
    GLuint m_texY  = 0;  // Luminance (full resolution)
    GLuint m_texCb = 0;  // Blue-difference chroma (half resolution)
    GLuint m_texCr = 0;  // Red-difference chroma (half resolution)

    // Video dimensions
    int m_width  = 0;
    int m_height = 0;

    // Playback state
    bool m_hasEnded = false;
    bool m_looping  = false;
    bool m_texturesCreated = false;

    // CPU-side buffers for decoded planes
    // We copy from plm_frame_t to these buffers in the callback,
    // then upload to GPU in the main path
    std::vector<uint8_t> m_bufferY;
    std::vector<uint8_t> m_bufferCb;
    std::vector<uint8_t> m_bufferCr;

    // Flag indicating we have a new frame to upload
    bool m_pendingUpload = false;

    /*!*************************************************************************
    \brief Static callback invoked by pl_mpeg when a video frame is decoded.
           Copies plane data to CPU buffers for later GPU upload.
    ***************************************************************************/
    static void OnVideoDecoded(plm_t* self, plm_frame_t* frame, void* user);

    /*!*************************************************************************
    \brief Upload decoded frame data from CPU buffers to GPU textures.
    ***************************************************************************/
    void UploadFrame();

    /*!*************************************************************************
    \brief Create the three YUV plane textures with the given dimensions.
           Called once when the first frame is decoded.
    \param width Video width in pixels
    \param height Video height in pixels
    ***************************************************************************/
    void CreateTextures(int width, int height);

    /*!*************************************************************************
    \brief Destroy OpenGL textures and reset handles.
    ***************************************************************************/
    void DestroyTextures();

    // -- Audio Support --
    static void OnAudioDecoded(plm_t* self, plm_samples_t* samples, void* user);
    FMOD_RESULT FMOD_ReadCallback(void* buffer, unsigned int lengthBytes);
    static FMOD_RESULT __stdcall FMODSourceCallback(struct FMOD_SOUND* sound, void* data, unsigned int datalen);

    FMOD::Sound* m_audioStream = nullptr;
    FMOD::Channel* m_audioChannel = nullptr;

    // Audio Ring Buffer
    std::vector<float> m_audioBuffer;
    size_t m_writeCursor = 0;
    size_t m_readCursor = 0;
    mutable std::mutex m_audioMutex;
    int m_sampleRate = 44100;
};
