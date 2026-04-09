#version 450 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;
layout (binding = 0) uniform sampler2D u_ScreenTex;

// tweakable exposure
uniform bool enabled;
uniform float u_Exposure;
uniform float u_Gamma; 
void main()
{

    vec3 hdrColor = texture(u_ScreenTex, TexCoords).rgb;

    // 🔹 Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * u_Exposure);

    // 🔹 Gamma correction
    mapped = pow(mapped, vec3(1.0 / u_Gamma));

    if (enabled)
    {
        FragColor = vec4(mapped, 1.0);
    }
    else
    {
        FragColor = vec4(hdrColor, 1.0);
    }
}
