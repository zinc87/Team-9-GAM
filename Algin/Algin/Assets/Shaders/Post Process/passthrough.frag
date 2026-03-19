#version 450 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;
layout (binding = 0) uniform sampler2D u_ScreenTex;

void main()
{
    FragColor = texture(u_ScreenTex, TexCoords);
}
