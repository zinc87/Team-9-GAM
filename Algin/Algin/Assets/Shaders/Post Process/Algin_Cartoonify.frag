#version 450 core

layout (location = 0) out vec4 FragColor;
in vec2 TexCoords;

layout (binding = 0) uniform sampler2D u_ScreenTex;

const float Threshold = 0.05;

void main()
{
    vec4 scene = texture(u_ScreenTex, TexCoords);
    vec3 originalColor = scene.rgb;

    // 1. Get the perceived brightness (Luminance)
    float luma = dot(originalColor, vec3(0.2126, 0.7152, 0.0722));

    // 2. Round the brightness to the nearest 0.1
    float roundedLuma = round(luma / Threshold) * Threshold;

    // 3. Apply the ratio to the original color
    // We use max() to avoid division by zero in pure black areas
    vec3 finalColor = originalColor * (roundedLuma / max(luma, 0.0001));

    // 4. Optional: Saturation Clamp
    // If colors feel too "neon" after rounding, pull them back toward gray
    // float gray = dot(finalColor, vec3(0.333));
    // finalColor = mix(finalColor, vec3(gray), 0.05);

    FragColor = vec4(clamp(finalColor, 0.0, 1.0), 1.0);
}