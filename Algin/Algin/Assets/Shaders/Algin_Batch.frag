#version 450 core

layout (location = 1) in vec4 vColor;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) flat in float vTexIndex;  // FLAT float

out vec4 FragColor;

uniform sampler2D uTextures[32];

void main()
{

    // --- Convert float index to int safely ---
    int idx = clamp(int(floor(vTexIndex + 0.5)), -1, 31);

    if (idx < 0) 
    {
        // Pure color mode (no texture sampling)
        FragColor = vColor;
    }
    else
    {
    // --- Sample texture ---
    vec4 texColor = texture(uTextures[idx], vTexCoord);

    // --- Detect font (single channel RED) ---
    // Fonts are stored as GL_RED (only .r has data)
    // For UI textures (RGBA), .rgb will contain actual color
    if (texColor.g == 0.0 && texColor.b == 0.0)
    {
        // Font rendering: use RED as alpha mask
        FragColor = vec4(vColor.rgb, texColor.r * vColor.a);
    }
    else
    {
        // Normal UI quad rendering
        FragColor = texColor * vColor;
    }
    }
    // --- Optional discard for full transparency ---
    if (FragColor.a <= 0.01)
        discard;
    
}