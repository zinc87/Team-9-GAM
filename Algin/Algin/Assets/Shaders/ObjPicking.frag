#version 450 core
layout(location = 0) out uvec2 outId;   // writes to GL_COLOR_ATTACHMENT0 (RG32UI)
uniform uvec2 uId64;                    // {low32, high32}

void main() {
    outId = uId64;
}

