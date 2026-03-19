#version 450 core

layout (location = 1) in vec4 vColor;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) flat in float vTexIndex;  // FLAT float

out vec4 FragColor;

uniform sampler2D uTextures[32];

void main() {
    // Solid-color quad sentinel (if you use -1)
    if (vTexIndex < 0.0) {
        FragColor = vColor;
        return;
    }

    // Robust float→int: round and clamp
    int idx = clamp(int(floor(vTexIndex + 0.5)), 0, 31);

    vec4 texColor = texture(uTextures[idx], vTexCoord);
    FragColor = texColor;

    // Optional alpha test cutout (uncomment if needed for leaf sprites, etc.)
    // if (FragColor.a < 0.05) discard;
}
