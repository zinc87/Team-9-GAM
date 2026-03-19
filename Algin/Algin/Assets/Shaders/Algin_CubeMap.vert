#version 450 core
layout(location = 0) in vec3 aPos;

out vec3 vTexCoord;

uniform mat4 P; // projection
uniform mat4 V; // view (but without translation!)

void main()
{
    // Remove translation from view matrix (so skybox follows camera)

    mat4 rotView = mat4(mat3(V));
    rotView = transpose(rotView);

    vec4 pos = P * rotView * vec4(aPos, 1.0);
    gl_Position = pos.xyww;

    vTexCoord = aPos; // use position as texture direction
}