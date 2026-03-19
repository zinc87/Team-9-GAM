/*!*****************************************************************************
\file VideoTexture.cpp
\brief Implementation of the VideoTexture class for MPEG-1 video playback.
       Uses pl_mpeg for decoding and uploads YUV planes to OpenGL textures.
*******************************************************************************/
#include "pch.h"
#include "VideoTexture.h"

// VideoTexture.h includes pl_mpeg.h, which provides full type definitions


//=============================================================================
// Construction / Destruction
//=============================================================================

VideoTexture::VideoTexture()
{
    // All members initialized in class definition
}

VideoTexture::~VideoTexture()
{
    Unload();
}

VideoTexture::VideoTexture(VideoTexture&& other) noexcept
    : m_decoder(other.m_decoder)
    , m_texY(other.m_texY)
    , m_texCb(other.m_texCb)
    , m_texCr(other.m_texCr)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_hasEnded(other.m_hasEnded)
    , m_looping(other.m_looping)
    , m_texturesCreated(other.m_texturesCreated)
    , m_bufferY(std::move(other.m_bufferY))
    , m_bufferCb(std::move(other.m_bufferCb))
    , m_bufferCr(std::move(other.m_bufferCr))
    , m_pendingUpload(other.m_pendingUpload)
{
    // Null out the moved-from object to prevent double-free
    other.m_decoder = nullptr;
    other.m_texY = other.m_texCb = other.m_texCr = 0;
    other.m_texturesCreated = false;

    // Re-register callback with new 'this' pointer
    if (m_decoder)
    {
        plm_set_video_decode_callback(m_decoder, OnVideoDecoded, this);
    }
}

VideoTexture& VideoTexture::operator=(VideoTexture&& other) noexcept
{
    if (this != &other)
    {
        Unload();

        m_decoder = other.m_decoder;
        m_texY = other.m_texY;
        m_texCb = other.m_texCb;
        m_texCr = other.m_texCr;
        m_width = other.m_width;
        m_height = other.m_height;
        m_hasEnded = other.m_hasEnded;
        m_looping = other.m_looping;
        m_texturesCreated = other.m_texturesCreated;
        m_bufferY = std::move(other.m_bufferY);
        m_bufferCb = std::move(other.m_bufferCb);
        m_bufferCr = std::move(other.m_bufferCr);
        m_pendingUpload = other.m_pendingUpload;

        other.m_decoder = nullptr;
        other.m_texY = other.m_texCb = other.m_texCr = 0;
        other.m_texturesCreated = false;

        if (m_decoder)
        {
            plm_set_video_decode_callback(m_decoder, OnVideoDecoded, this);
        }
    }
    return *this;
}

//=============================================================================
// Public Interface
//=============================================================================

bool VideoTexture::Load(const std::string& path)
{
    // Clean up any existing video first
    Unload();

    // Create decoder from file
    // plm_create_with_filename returns nullptr on failure
    m_decoder = plm_create_with_filename(path.c_str());
    if (!m_decoder)
    {
        AG_CORE_WARN("VideoTexture: Failed to load video file: {}", path);
        return false;
    }

    // Get video dimensions from the decoder
    m_width  = plm_get_width(m_decoder);
    m_height = plm_get_height(m_decoder);

    // Verify dimensions
    if (m_width <= 0 || m_height <= 0)
    {
        AG_CORE_WARN("VideoTexture: Invalid video dimensions: {}x{}", m_width, m_height);
        plm_destroy(m_decoder);
        m_decoder = nullptr;
        return false;
    }

    // -- Audio Setup --
    plm_set_audio_enabled(m_decoder, true); // Enable audio
    plm_set_audio_decode_callback(m_decoder, OnAudioDecoded, this);

    m_sampleRate = plm_get_samplerate(m_decoder);
    AG_CORE_INFO("VideoTexture: Audio Sample Rate: {}", m_sampleRate);

    // Initial Buffer: 4 seconds of stereo audio (safe margin)
    // 2 channels * 4 seconds
    size_t sampleCount = static_cast<size_t>(m_sampleRate) * 2 * 4; 
    m_audioBuffer.resize(sampleCount);
    std::fill(m_audioBuffer.begin(), m_audioBuffer.end(), 0.0f);
    m_readCursor = 0;
    m_writeCursor = 0;

    // FMOD Stream Creation
    FMOD::System* fmodSystem = AUDIOSYSTEM.GetCoreSystem();
    if (fmodSystem)
    {
        FMOD_CREATESOUNDEXINFO exinfo;
        std::memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.numchannels = 2; // MPEG-1 is stereo/mono, assuming stereo for simplicity (pl_mpeg outputs stereo)
        exinfo.defaultfrequency = m_sampleRate;
        exinfo.format = FMOD_SOUND_FORMAT_PCMFLOAT;
        exinfo.length = (unsigned int)(plm_get_duration(m_decoder) * m_sampleRate * 2 * sizeof(float)); 
        exinfo.pcmreadcallback = FMODSourceCallback;
        exinfo.userdata = this;

        // Open as a user-created stream
        FMOD_RESULT res = fmodSystem->createSound(nullptr, FMOD_OPENUSER | FMOD_LOOP_NORMAL | FMOD_CREATESTREAM, &exinfo, &m_audioStream);
        if (res == FMOD_OK)
        {
            // Start playing (paused initially?) No, play immediately to sync
            fmodSystem->playSound(m_audioStream, nullptr, false, &m_audioChannel);
        }
        else
        {
            AG_CORE_WARN("VideoTexture: Failed to create FMOD stream. Error: {}", (int)res);
        }
    }

    // Set up video decode callback
    plm_set_video_decode_callback(m_decoder, OnVideoDecoded, this);

    // Pre-allocate CPU buffers for plane data
    m_bufferY.resize(static_cast<size_t>(m_width) * m_height);
    m_bufferCb.resize(static_cast<size_t>(m_width / 2) * (m_height / 2));
    m_bufferCr.resize(static_cast<size_t>(m_width / 2) * (m_height / 2));

    m_hasEnded = false;
    m_pendingUpload = false;

    AG_CORE_WARN("VideoTexture: Loaded {} ({}x{}, {:.2f}s)", 
                 path, m_width, m_height, plm_get_duration(m_decoder));

    return true;
}

void VideoTexture::Unload()
{
    // Release Audio
    // CRITICAL FIX: Application::Free() might destroy AUDIOSYSTEM before this runs.
    // If GetCoreSystem() returns nullptr, the system is already gone and our resources
    // are implicitly freed (or at least invalid to touch).
    if (AUDIOSYSTEM.GetCoreSystem() && m_audioStream)
    {
        if (m_audioChannel) {
            m_audioChannel->stop();
            m_audioChannel = nullptr;
        }
        
        m_audioStream->release();
        m_audioStream = nullptr;
    }
    // Just ensure they are null if we skipped the block
    m_audioStream = nullptr;
    m_audioChannel = nullptr;

    // Destroy decoder
    if (m_decoder)
    {
        plm_destroy(m_decoder);
        m_decoder = nullptr;
    }

    // Destroy GPU textures
    DestroyTextures();

    // Clear CPU buffers
    m_bufferY.clear();
    m_bufferCb.clear();
    m_bufferCr.clear();
    
    // Clear audio buffer
    std::lock_guard<std::mutex> lock(m_audioMutex);
    m_audioBuffer.clear();

    // Reset state
    m_width = m_height = 0;
    m_hasEnded = false;
    m_pendingUpload = false;
}

// ... helper for ring buffer writing ...
void VideoTexture::OnAudioDecoded(plm_t* /*self*/, plm_samples_t* samples, void* user)
{
    VideoTexture* video = static_cast<VideoTexture*>(user);
    if (!video) return;

    // samples->count is number of frames (pairs of samples if stereo)
    // samples->interleaved points to float array of size count * 2
    size_t sampleCount = samples->count * 2;
    const float* src = samples->interleaved;

    std::lock_guard<std::mutex> lock(video->m_audioMutex);
    
    if (video->m_audioBuffer.empty()) return;

    size_t bufferSize = video->m_audioBuffer.size();
    size_t writePos = video->m_writeCursor;

    // Simple copy with wrap-around
    for (size_t i = 0; i < sampleCount; ++i)
    {
        video->m_audioBuffer[writePos] = src[i];
        writePos = (writePos + 1) % bufferSize;
    }

    video->m_writeCursor = writePos;
}

FMOD_RESULT __stdcall VideoTexture::FMODSourceCallback(struct FMOD_SOUND* sound, void* data, unsigned int datalen)
{
    VideoTexture* video = nullptr;
    ((FMOD::Sound*)sound)->getUserData((void**)&video);

    if (!video)
    {
        // Silence if no object
        std::memset(data, 0, datalen);
        return FMOD_OK;
    }

    float* outBuffer = (float*)data;
    unsigned int samplesToRead = datalen / sizeof(float); // Total floats needed

    std::lock_guard<std::mutex> lock(video->m_audioMutex);

    if (video->m_audioBuffer.empty())
    {
        std::memset(data, 0, datalen);
        return FMOD_OK;
    }

    size_t bufferSize = video->m_audioBuffer.size();
    size_t readPos = video->m_readCursor;
    size_t writePos = video->m_writeCursor;

    for (unsigned int i = 0; i < samplesToRead; ++i)
    {
        // Check if buffer underflow (read caught up to write)
        // Note: Simple check. For a robust system we track 'available' count.
        // But for this, if read == write, we assume empty.
        // Wait, if write wrapped around and equals read, it's FULL. 
        // Need a better 'available' check.
        // Let's rely on write being ahead. 
        // If pointers are equal, buffer is empty (unless full, but we assume we consume faster or buffer large enough).
        
        // Actually, distance logic:
        size_t available = (writePos >= readPos) ? (writePos - readPos) : (bufferSize - readPos + writePos);
        
        if (available == 0)
        {
            outBuffer[i] = 0.0f; // Silence
            continue;
        }

        outBuffer[i] = video->m_audioBuffer[readPos];
        readPos = (readPos + 1) % bufferSize;
    }

    video->m_readCursor = readPos;

    return FMOD_OK;
}

void VideoTexture::Update(float dt)
{
    if (!m_decoder || m_hasEnded)
        return;

    // Decode video for the given time delta
    // This will trigger OnVideoDecoded callback for each decoded frame
    plm_decode(m_decoder, static_cast<double>(dt));

    // Check if video has ended
    if (plm_has_ended(m_decoder))
    {
        if (m_looping)
        {
            // Rewind to beginning for seamless loop
            Rewind();
        }
        else
        {
            m_hasEnded = true;
        }
    }

    // Upload any pending frame data to GPU
    // We do this on the main path (not in callback) to ensure OpenGL context is current
    if (m_pendingUpload)
    {
        UploadFrame();
        m_pendingUpload = false;
    }
}

void VideoTexture::BindYUV()
{
    // Bind Y plane to texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texY);

    // Bind Cb plane to texture unit 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texCb);

    // Bind Cr plane to texture unit 2
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_texCr);

    // Reset active texture to unit 0 (good practice)
    glActiveTexture(GL_TEXTURE0);
}

void VideoTexture::Rewind()
{
    if (m_decoder)
    {
        plm_rewind(m_decoder);
        m_hasEnded = false;
    }
}

//=============================================================================
// Private Implementation
//=============================================================================

void VideoTexture::OnVideoDecoded(plm_t* /*self*/, plm_frame_t* frame, void* user)
{
    // 'user' is our VideoTexture instance
    VideoTexture* video = static_cast<VideoTexture*>(user);
    if (!video || !frame)
        return;

    // pl_mpeg decodes to YCbCr 4:2:0 format:
    // - Y plane: full resolution, single byte per pixel
    // - Cb/Cr planes: half resolution in each dimension

    // Copy Y plane data
    // frame->y.width is the STRIDE, which may be larger than m_width (padding)
    // We only copy the visible m_width pixels per row
    const int copyW = video->m_width;
    const int copyH = video->m_height;
    const int strideY = frame->y.width;

    // Safety check
    if (video->m_bufferY.size() < static_cast<size_t>(copyW) * copyH)
        return;

    for (int row = 0; row < copyH; ++row)
    {
        std::memcpy(
            video->m_bufferY.data() + row * copyW,
            frame->y.data + row * strideY,
            copyW
        );
    }

    // Copy Cb plane data (half resolution)
    const int copyW_Half = copyW / 2;
    const int copyH_Half = copyH / 2;
    const int strideCb = frame->cb.width;

    for (int row = 0; row < copyH_Half; ++row)
    {
        std::memcpy(
            video->m_bufferCb.data() + row * copyW_Half,
            frame->cb.data + row * strideCb,
            copyW_Half
        );
    }

    // Copy Cr plane data (half resolution)
    const int strideCr = frame->cr.width;

    for (int row = 0; row < copyH_Half; ++row)
    {
        std::memcpy(
            video->m_bufferCr.data() + row * copyW_Half,
            frame->cr.data + row * strideCr,
            copyW_Half
        );
    }

    // Mark that we have new frame data to upload
    video->m_pendingUpload = true;
}

void VideoTexture::CreateTextures(int width, int height)
{
    if (m_texturesCreated)
        return;

    // Generate texture handles
    glGenTextures(1, &m_texY);
    glGenTextures(1, &m_texCb);
    glGenTextures(1, &m_texCr);

    // Helper lambda to configure a single-channel texture
    auto configureTexture = [](GLuint tex, int w, int h)
    {
        glBindTexture(GL_TEXTURE_2D, tex);

        // Allocate storage for R8 format (single channel, 8-bit)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

        // Use linear filtering for smooth playback
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Clamp to edge to avoid border artifacts
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    };

    // Y plane: full resolution
    configureTexture(m_texY, width, height);

    // Cb/Cr planes: half resolution (4:2:0 chroma subsampling)
    configureTexture(m_texCb, width / 2, height / 2);
    configureTexture(m_texCr, width / 2, height / 2);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_texturesCreated = true;
}

void VideoTexture::UploadFrame()
{
    // Create textures on first frame (ensures we have valid dimensions)
    if (!m_texturesCreated)
    {
        CreateTextures(m_width, m_height);
    }

    // Upload Y plane
    glBindTexture(GL_TEXTURE_2D, m_texY);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height,
                    GL_RED, GL_UNSIGNED_BYTE, m_bufferY.data());

    // Upload Cb plane (half resolution)
    glBindTexture(GL_TEXTURE_2D, m_texCb);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width / 2, m_height / 2,
                    GL_RED, GL_UNSIGNED_BYTE, m_bufferCb.data());

    // Upload Cr plane (half resolution)
    glBindTexture(GL_TEXTURE_2D, m_texCr);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width / 2, m_height / 2,
                    GL_RED, GL_UNSIGNED_BYTE, m_bufferCr.data());

    glBindTexture(GL_TEXTURE_2D, 0);
}

void VideoTexture::DestroyTextures()
{
    if (m_texY)
    {
        glDeleteTextures(1, &m_texY);
        m_texY = 0;
    }
    if (m_texCb)
    {
        glDeleteTextures(1, &m_texCb);
        m_texCb = 0;
    }
    if (m_texCr)
    {
        glDeleteTextures(1, &m_texCr);
        m_texCr = 0;
    }

    m_texturesCreated = false;
}
