#version 450 core

layout(location = 0) in vec3 aPos;       // position
layout(location = 1) in vec3 aNormal;    // normal
layout(location = 2) in vec2 aTex;       // texcoords
layout(location = 3) in vec3 aTangent;   // tangent
layout(location = 4) in vec3 aBitangent; // bitangent
layout(location = 5) in ivec4 aBoneID;
layout(location = 6) in vec4 aWeights;

uniform mat4 P;   // projection
uniform mat4 V;   // view
uniform mat4 M;   // model
uniform mat3 uNormalMat;

uniform mat4 uBones[250];
uniform int uNumBones;

out VS_OUT {
    vec3 WorldPos;
    vec3 NormalWS;
    vec2 TexCoord;
    mat3 TBN;
} vs;


mat4 buildSkinMatrix(ivec4 IDs, vec4 weight) {
    // Clamp indices to available range to avoid invalid indexing
    int i0 = clamp(IDs.x, 0, uNumBones - 1);
    int i1 = clamp(IDs.y, 0, uNumBones - 1);
    int i2 = clamp(IDs.z, 0, uNumBones - 1);
    int i3 = clamp(IDs.w, 0, uNumBones - 1);

    // Blend the 4 bone matrices (Linear Blend Skinning)
    mat4 skin = mat4(0.0);
    skin += weight.x * uBones[i0];
    skin += weight.y * uBones[i1];
    skin += weight.z * uBones[i2];
    skin += weight.w * uBones[i3];
    return skin;
}

void main()
{   

    // 1) Build skin matrix from influences
    mat4 skin = buildSkinMatrix(aBoneID, aWeights);

    // 2) Skin position & normal in MODEL space
    vec4 posSkinned_MS = skin * vec4(aPos, 1.0);
    
    // world position
    vec4 wp = M * posSkinned_MS;
    vs.WorldPos = wp.xyz;

    // normal in world space
    mat3 skin3 = mat3(skin);
    vec3 nSkinned_MS = normalize(skin3 * aNormal);
    vec3 N = normalize(uNormalMat * nSkinned_MS);

    // tangent in world space
    vec3 tSkinned_MS = skin3 * aTangent;
    vec3 T = normalize(uNormalMat * tSkinned_MS);
    T = normalize(T - dot(N, T) * N);

    // bitangent using handedness (aTangent.w)
    vec3 B = normalize(cross(N, T));

    // build TBN matrix
    vs.TBN = mat3(T, B, N);
    vs.NormalWS = N;

    vs.TexCoord = aTex;

    gl_Position = P * V * wp;
}
