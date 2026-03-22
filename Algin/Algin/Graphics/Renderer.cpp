#include "pch.h"
#include "Renderer.h"

bool AABB::contains(glm::vec3 point)
{
    return (point.x >= min.x && point.x <= max.x) &&
        (point.y >= min.y && point.y <= max.y) &&
        (point.z >= min.z && point.z <= max.z);
}

AABB AABB::transform(glm::mat4 m2w)
{
    glm::vec3 corners[8] = {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {min.x, max.y, min.z},
        {max.x, max.y, min.z},
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {min.x, max.y, max.z},
        {max.x, max.y, max.z}
    };

    glm::vec3 newMin(std::numeric_limits<float>::max());
    glm::vec3 newMax(-std::numeric_limits<float>::max());

    // transform each corner, expand bounds
    for (int i = 0; i < 8; ++i) {
        glm::vec4 worldPos = m2w * glm::vec4(corners[i], 1.0f);
        glm::vec3 p = glm::vec3(worldPos);

        newMin = glm::min(newMin, p);
        newMax = glm::max(newMax, p);
    }

    return AABB(newMin, newMax);
}


SkyBoxRenderer::SkyBoxRenderer()
{
    // Generate VAO, VBO, EBO
    glGenVertexArrays(1, &SB_VAO);
    glGenBuffers(1, &SB_VBO);
    glGenBuffers(1, &SB_EBO);

    glBindVertexArray(SB_VAO);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, SB_VBO); glBufferData(GL_ARRAY_BUFFER, skyboxCorner.size() * sizeof(glm::vec3), skyboxCorner.data(), GL_STATIC_DRAW);


    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SB_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, skyboxIndices.size() * sizeof(unsigned int), skyboxIndices.data(), GL_STATIC_DRAW);

    // VAO
    // position (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0); 

    glBindVertexArray(0);

    SHADERMANAGER.Use("Algin_CubeMap");
    locP = glGetUniformLocation(SHADERMANAGER.GetShaderProgram(), "P");
    locV = glGetUniformLocation(SHADERMANAGER.GetShaderProgram(), "V");
    locSkyBox = glGetUniformLocation(SHADERMANAGER.GetShaderProgram(), "skybox");
}

void SkyBoxRenderer::RenderSkyBox(const glm::mat4& P, const glm::mat4& V, size_t cubemap_texture)
{ 
	SHADERMANAGER.Use("Algin_CubeMap");

    if (locP != -1) glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(P));
    if (locV != -1) glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(V));

    // Depth setup
    glDepthFunc(GL_LEQUAL);  // Let fragments at depth == 1.0 pass
    glDepthMask(GL_FALSE);   // Disable depth writes

    // Bind cubemap texture
    glActiveTexture(GL_TEXTURE0);
    auto asset = AssetManager::GetInstance().GetAsset(cubemap_texture).lock();
    auto tex_asset = std::dynamic_pointer_cast<CubeMapAsset>(asset);
    GLuint tex_id = 0;
    if (tex_asset)
    {
        tex_id = tex_asset->textureID;
    }
    else AG_CORE_WARN("No Texture for skybox");
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
    glUniform1i(locSkyBox, 0);
    // Draw cube (6 faces -> 36 indices)
    glBindVertexArray(SB_VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);    // Restore depth writes
    glDepthFunc(GL_LESS);    // Restore default

}


AG::RenderPass::RenderPass()
{
    passes[PASS_TRANSLUCENT].shader = "Algin_3D";
    passes[PASS_OPAQUE].shader = "Algin_3D";
    passes[PASS_SKYBOX].shader = "Algin_3D";
}

void AG::RenderPass::AddToPass(PASS_TYPE pt, std::function<void()> renderFunc)
{
    passes[pt].funcPtr.push_back(renderFunc);
}

void AG::RenderPass::Render()
{
    for (auto& pass : passes)
    {
        SHADERMANAGER.Use(pass.shader);
        for (auto& func : pass.funcPtr)
        {
            func();
        }
        pass.funcPtr.clear(); // flush
    }
}


void AG::PostProcesser::Init()
{
    auto [width, height] = AGWindow::GetInstance().getViewportSize();

    // --- HDR framebuffer ---
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glGenTextures(1, &hdrColorTex);
    glBindTexture(GL_TEXTURE_2D, hdrColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
        GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, hdrColorTex, 0);

    // Depth renderbuffer
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        AG_CORE_WARN("HDR framebuffer not complete!");

    // --- Ping-pong buffers ---
    glGenFramebuffers(1, &pingFBO);
    glGenFramebuffers(1, &pongFBO);
    glGenTextures(1, &pingTex);
    glGenTextures(1, &pongTex);

    GLuint fbos[2] = { pingFBO, pongFBO };
    GLuint texs[2] = { pingTex, pongTex };
    
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
        glBindTexture(GL_TEXTURE_2D, texs[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
            GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, texs[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            AG_CORE_WARN("Ping-pong framebuffer {} not complete!", i);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint grayscale = LoadShader("Assets/Shaders/Post Process/Algin_Greyscale.vert", "Assets/Shaders/Post Process/Algin_Greyscale.frag");
    PP_MAP["grayscale"] = grayscale;


    GLuint blur = LoadShader("Assets/Shaders/Post Process/Algin_Blur.vert", "Assets/Shaders/Post Process/Algin_Blur.frag");
    PP_MAP["blur"] = blur;

    GLuint final = LoadShader("Assets/Shaders/Post Process/fullscreen.vert", "Assets/Shaders/Post Process/tonemap.frag");
    PP_MAP["final"] = final;

    finalPassthrough = LoadShader("Assets/Shaders/Post Process/fullscreen.vert", "Assets/Shaders/Post Process/passthrough.frag");
    if (!finalPassthrough)
        AG_CORE_WARN("Failed to load passthrough post-process shader.");

    GLuint cartoon = LoadShader("Assets/Shaders/Post Process/Algin_Cartoonify.vert", "Assets/Shaders/Post Process/Algin_Cartoonify.frag");
    PP_MAP["cartoon"] = cartoon;
}

void AG::PostProcesser::BeginScene()
{
    auto [width, height] = AGWindow::GetInstance().getWindowSize();

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void AG::PostProcesser::EndScene()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void AG::PostProcesser::Apply()
{
    auto [width, height] = AGWindow::GetInstance().getWindowSize();

    bool usePing = true;
    GLuint inputTex = hdrColorTex;

    while (!process_queue.empty()) {
        std::string effect = process_queue.front();
        process_queue.pop();
        if (PP_MAP.find(effect) == PP_MAP.end()) {
            AG_CORE_WARN("Post-process effect not found: {}", effect);
            continue;
        }

        GLuint shader = PP_MAP[effect];
        glUseProgram(shader);

        GLuint targetFBO = usePing ? pingFBO : pongFBO;
        GLuint targetTex = usePing ? pingTex : pongTex;

        glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTextureUnit(0, inputTex);
        glUniform1i(glGetUniformLocation(shader, "u_ScreenTex"), 0);

        RenderFullscreenQuad(); 

        inputTex = targetTex;
        usePing = !usePing;
    }
    // --- Final pass: render to backbuffer ---
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT); 

    auto finalIt = PP_MAP.find("final");
    if (finalIt != PP_MAP.end()) {
        GLuint finalShader = finalIt->second;
        if (!m_enableTonemap && finalPassthrough != 0)
            finalShader = finalPassthrough;

        bool usingTonemap = (finalShader == finalIt->second);

        glUseProgram(finalShader);
        glBindTextureUnit(0, inputTex);
        glUniform1i(glGetUniformLocation(finalShader, "u_ScreenTex"), 0);

        if (usingTonemap)
        {
            GLint enabledLoc = glGetUniformLocation(finalShader, "enabled");
            if (enabledLoc != -1)
                glUniform1i(enabledLoc, m_enableTonemap ? 1 : 0);

            if (CameraManager::GetInstance().getCurrentCamera().lock())
            {
                glUniform1f(glGetUniformLocation(finalShader, "u_Exposure"),
                    CameraManager::GetInstance().getCurrentCamera().lock()->getExposure());
                glUniform1f(glGetUniformLocation(finalShader, "u_Gamma"), gammaScaling);
            }
            else
            {
                glUniform1f(glGetUniformLocation(finalShader, "u_Exposure"), 1.0f);
                glUniform1f(glGetUniformLocation(finalShader, "u_Gamma"), gammaScaling);
            }
        }
        else
        {
            // Still send gamma to passthrough shader so cutscenes respect gamma
            glUniform1f(glGetUniformLocation(finalShader, "u_Gamma"), gammaScaling);
        }

        RenderFullscreenQuad();
    }
}

void AG::PostProcesser::ApplyOverlay(int vpX, int vpY, int vpW, int vpH)
{
    if (vpW <= 0 || vpH <= 0)
        return;

    auto it = PP_MAP.find("final");
    if (it == PP_MAP.end())
        return;

    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);

    GLboolean scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
    GLint prevScissor[4];
    glGetIntegerv(GL_SCISSOR_BOX, prevScissor);

    GLboolean depthEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);

    glViewport(vpX, vpY, vpW, vpH);
    glEnable(GL_SCISSOR_TEST);
    glScissor(vpX, vpY, vpW, vpH);

    GLuint finalShader = it->second;
    if (!m_enableTonemap && finalPassthrough != 0)
        finalShader = finalPassthrough;

    bool usingTonemap = (finalShader == it->second);

    glUseProgram(finalShader);
    glBindTextureUnit(0, hdrColorTex);
    glUniform1i(glGetUniformLocation(finalShader, "u_ScreenTex"), 0);

    if (usingTonemap)
    {
        GLint enabledLoc = glGetUniformLocation(finalShader, "enabled");
        if (enabledLoc != -1)
            glUniform1i(enabledLoc, m_enableTonemap ? 1 : 0);

        if (CameraManager::GetInstance().getCurrentCamera().lock())
        {
            glUniform1f(glGetUniformLocation(finalShader, "u_Exposure"),
                CameraManager::GetInstance().getCurrentCamera().lock()->getExposure());
            glUniform1f(glGetUniformLocation(finalShader, "u_Gamma"), gammaScaling);
        }
        else
        {
            glUniform1f(glGetUniformLocation(finalShader, "u_Exposure"), 1.0f);
            glUniform1f(glGetUniformLocation(finalShader, "u_Gamma"), gammaScaling);
        }
    }
    else
    {
        // Still send gamma to passthrough shader so cutscenes respect gamma
        glUniform1f(glGetUniformLocation(finalShader, "u_Gamma"), gammaScaling);
    }

    glDisable(GL_DEPTH_TEST);
    RenderFullscreenQuad();
    if (depthEnabled) glEnable(GL_DEPTH_TEST);

    glUseProgram(0);

    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    glScissor(prevScissor[0], prevScissor[1], prevScissor[2], prevScissor[3]);
    if (!scissorEnabled)
        glDisable(GL_SCISSOR_TEST);
}



void AG::PostProcesser::AddEffect(const std::string& name, GLuint shader)
{
    PP_MAP[name] = shader;
}

void AG::PostProcesser::QueueEffect(const std::string& name)
{
    process_queue.push(name);
}


GLuint AG::PostProcesser::LoadShader(const std::string& vertexPath, const std::string& fragmentPath)
{
    // --- 1. Read shader source files ---
    auto readFile = [](const std::string& path) -> std::string {
        std::ifstream file(path);
        if (!file.is_open()) {
            AG_CORE_WARN("Failed to open shader file: {}", path);
            return {};
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
        };

    std::string vertCode = readFile(vertexPath);
    std::string fragCode = readFile(fragmentPath);

    if (vertCode.empty() || fragCode.empty()) {
        AG_CORE_WARN("Shader source is empty! VS: {}, FS: {}", vertexPath, fragmentPath);
        return 0;
    }

    const char* vSrc = vertCode.c_str();
    const char* fSrc = fragCode.c_str();

    // --- 2. Compile shaders ---
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vSrc, nullptr);
    glCompileShader(vertex);

    GLint success;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetShaderInfoLog(vertex, 1024, nullptr, info);
        AG_CORE_WARN("Vertex shader compile error ({}):\n{}", vertexPath, info);
    }

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fSrc, nullptr);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetShaderInfoLog(fragment, 1024, nullptr, info);
        AG_CORE_WARN("Fragment shader compile error ({}):\n{}", fragmentPath, info);
    }

    // --- 3. Link program ---
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetProgramInfoLog(program, 1024, nullptr, info);
        AG_CORE_WARN("Shader program link error ({} + {}):\n{}", vertexPath, fragmentPath, info);
    }

    // --- 4. Cleanup ---
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

void AG::PostProcesser::OnResize(int width, int height)
{
    if (width == m_width && height == m_height)
        return; // no change

    m_width = width;
    m_height = height;

    // --- Resize HDR color buffer ---
    glBindTexture(GL_TEXTURE_2D, hdrColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
        0, GL_RGBA, GL_FLOAT, nullptr);

    // Resize renderbuffer for depth/stencil
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    // --- Resize ping-pong textures ---
    glBindTexture(GL_TEXTURE_2D, pingTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
        0, GL_RGBA, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, pongTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
        0, GL_RGBA, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    AG_CORE_WARN("PostProcesser resized to {}x{}", width, height);
}

void AG::PostProcesser::RenderFullscreenQuad()
{
     static GLuint quadVAO = 0;
    static GLuint quadVBO;

    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions   // texcoords
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
