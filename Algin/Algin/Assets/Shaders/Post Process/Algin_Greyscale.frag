
#version 450 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D u_ScreenTex;

void main()
{
    vec3 col = texture(u_ScreenTex, TexCoords).rgb;
    float gray = dot(col, vec3(0.299, 0.587, 0.114)); // Rec.601
    FragColor = vec4(vec3(gray), 1.0);
}
