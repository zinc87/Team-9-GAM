#version 450 core


layout(location=0) in vec3 aPos;
layout(location=1) in vec2 a_uv;

layout(location=2) in vec3 particlePos;
layout(location=3) in vec4 particleColor;
layout(location=4) in float particleSize;
layout(location=5) in float particleRot;

uniform mat4 P;   // projection
uniform mat4 V;   // view

uniform vec3 u_position;
uniform float u_size;
uniform float u_rotation;

out vec2 v_uv;
out vec4 v_color;

void main(){
	
    vec3 camRight = vec3(V[0][0], V[1][0], V[2][0]);
    vec3 camUp    = vec3(V[0][1], V[1][1], V[2][1]);

    float c = cos(particleRot), s = sin(particleRot);
    vec2 rot = vec2(c * aPos.x - s * aPos.y, s * aPos.x + c * aPos.y);

	vec3 worldPos = particlePos + (camRight * rot.x + camUp * rot.y) * particleSize;

	gl_Position = P * V * vec4(worldPos, 1.0);
	v_uv = a_uv;
	v_color = particleColor;
}