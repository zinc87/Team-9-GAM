#version 450 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;
layout (binding = 0) uniform sampler2D u_ScreenTex;

uniform float u_Gamma;

void main()
{
    vec3 color = texture(u_ScreenTex, TexCoords).rgb;

    // Apply gamma correction even when tonemapping is off
    if (u_Gamma > 0.0)
        color = pow(color, vec3(1.0 / u_Gamma));

    FragColor = vec4(color, 1.0);
}
