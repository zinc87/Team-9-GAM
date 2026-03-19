#version 450 core

layout(location=0) in vec3 aPos;

uniform mat4 P;   // projection
uniform mat4 V;   // view
uniform mat4 M;   // model

void main() { 
	gl_Position = P * V * M * vec4(aPos,1.0); 
}