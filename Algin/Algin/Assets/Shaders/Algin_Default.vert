#version 450 core

// Attributes
layout (location = 0) in vec3 aPos;        // world-space position (you already apply TRS on CPU)
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aTexIndex;  // must remain float

// Varyings
layout (location = 1) out vec4 vColor;
layout (location = 2) out vec2 vTexCoord;
layout (location = 3) flat out float vTexIndex; // FLAT so it doesn't interpolate

// Uniforms
uniform mat4 P;   // Projection
uniform mat4 V;   // View

void main() {
    gl_Position = P * V * vec4(aPos, 1.0);
    vColor     = aColor;
    vTexCoord  = aTexCoord;
    vTexIndex  = aTexIndex;  // will be carried per-triangle without interpolation
}
