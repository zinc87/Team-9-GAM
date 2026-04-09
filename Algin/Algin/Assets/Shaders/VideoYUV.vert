#version 450 core

// VideoYUV Vertex Shader
// Passes through a fullscreen quad for video rendering.
// Matches the post-process shader convention used in the engine.

out vec2 TexCoords;

void main()
{
    // Generate a full-screen triangle using gl_VertexID (0, 1, 2)
    // This allows rendering without binding a Vertex Buffer Object.
    // Positions: (-1, -1), (3, -1), (-1, 3) covers the NDC [-1, 1] range.
    
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);

    TexCoords.x = (x + 1.0) * 0.5;
    TexCoords.y = 1.0 - ((y + 1.0) * 0.5); // Flip Y

    gl_Position = vec4(x, y, 0.0, 1.0);
}
