#version 450 core

layout(location = 0) in vec3 aPos;       // position
layout(location = 1) in vec3 aNormal;    // normal
layout(location = 2) in vec2 aTex;       // texcoords
layout(location = 3) in vec3 aTangent;   // tangent
layout(location = 4) in vec3 aBitangent; // bitangent

uniform mat4 P;   // projection
uniform mat4 V;   // view
uniform mat4 M;   // model
uniform mat3 uNormalMat;

out VS_OUT {
    vec3 WorldPos;
    vec3 NormalWS;
    vec2 TexCoord;
    mat3 TBN;
} vs;

void main()
{
    // world position
    vec4 wp = M * vec4(aPos, 1.0);
    vs.WorldPos = wp.xyz;

    // normal in world space
    mat3 normalMat = mat3(transpose(inverse(M)));
    vec3 N = normalize(normalMat * aNormal);
    vs.NormalWS = N;

    // tangent in world space
    vec3 T = normalize(normalMat * aTangent);
    T = normalize(T - dot(N,T) * N);

    // bitangent using handedness (aTangent.w)
    vec3 B = normalize(cross(N, T));

    // build TBN matrix
    vs.TBN = mat3(T, B, N);

    vs.TexCoord = aTex;
    gl_Position = P * V * wp;
}
