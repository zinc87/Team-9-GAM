#version 450 core

in vec2 v_uv;
in vec4 v_color;
out vec4 FragColor;

uniform sampler2D u_tex;

void main() {
	FragColor = v_color;
}