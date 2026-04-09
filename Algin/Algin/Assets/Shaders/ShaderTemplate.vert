#version 450 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTexCoord;
layout(location=2) in vec3 aNormal;
layout(location = 3) in vec3 aTangent;


uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out VS_OUT {
    vec3 WorldPos;
    vec3 NormalWS;
    vec2 TexCoord;
    mat3 TBN;
} vs;

void main() {

    // world position
    vec4 wp = M * vec4(aPos, 1.0);
    vs.WorldPos = wp.xyz;

    // normal in world space
    mat3 normalMat = mat3(transpose(inverse(M)));
    vec3 N = normalize(normalMat * aNormal);
    vs.NormalWS = N;

    vec3 T = normalize(normalMat * aTangent);
    T = normalize(T - dot(N,T) * N);

    vec3 B = normalize(cross(N, T));

    vs.TBN = mat3(T,B,N);


    vs.TexCoord = aTexCoord;
    gl_Position = P * V * wp;
}

