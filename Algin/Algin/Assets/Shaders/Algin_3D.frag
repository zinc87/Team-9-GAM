#version 450 core

out vec4 FragColor;


//PART OF MATERIAL
uniform vec4 uBaseColor;
uniform float uMetallic;
uniform float uRoughness;
uniform float uUvScale;
uniform float uTint;

//PART OF MATERIAL


in VS_OUT {
    vec3 WorldPos;
    vec3 NormalWS;
    vec2 TexCoord;
    mat3 TBN;
} fs;

struct Light {
    vec3 position;
    vec3 direction;
    float range;
    float intensity;
    float outerCutoff;
    float innerCutoff;
    int type;
};

struct Material {
    vec3  diffuseColor;
    vec3  specularColor;
    float shininess;
    int   hasDiffuseTex;
    int   hasNormalTex;
    int   hasSpecularTex;
};

const int MAX_LIGHTS = 16;
uniform int   uLightCount;
uniform Light uLights[MAX_LIGHTS];

uniform Material uMaterial;

uniform sampler2D uDiffuseTex; //albedo map
uniform sampler2D uNormalTex;  //normal map
//uniform sampler2D uSpecularTex;

uniform vec3 camPos;

/* ---- SHADOW ARRAYS ---- */
#define MAX_SHADOWED_LIGHTS 4
uniform int uNumShadowed;
uniform sampler2DShadow uShadowMap[MAX_SHADOWED_LIGHTS];
uniform mat4 uLightSpace[MAX_SHADOWED_LIGHTS];
uniform int uLightShadowIdx[MAX_SHADOWED_LIGHTS];

// ---- PBR helpers ----
const float PI = 3.14159265359;
float saturate(float x){ return clamp(x, 0.0, 1.0); }
vec3 ambientLight = vec3(0.1, 0.1, 0.1);

float D_GGX(float NdotH, float a) {
    float a2 = a * a;
    float d  = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

// Schlick-G for GGX
float G_Smith_GGX_Schlick(float NdotV, float NdotL, float a) {
    float k = (a + 1.0);
    k = (k * k) / 8.0;
    float gV = NdotV / (NdotV * (1.0 - k) + k);
    float gL = NdotL / (NdotL * (1.0 - k) + k);
    return gV * gL;
}

vec3 F_Schlick(vec3 F0, float VdotH) {
    // Standard Schlick Fresnel
    return F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);
}

float attenuation(float dist, float range)
{
    float att = 1.0 / (1.0 + 0.1 * dist + 0.01 * dist * dist);
    float fade = 1.0 - smoothstep(range * 0.9, range, dist); // smooth fade-out near edge
    return att * fade;
}


vec3 shadeLight(Light light, vec3 normal,vec3 albedo){
    
    vec3 Lvec = light.position - fs.WorldPos;
    float lightDist = length(Lvec);
    if (lightDist <= 1e-6) return vec3(0.0);

    vec3 N = normalize(normal);
    vec3 V = normalize(camPos - fs.WorldPos);
    vec3 L = normalize(Lvec);
    vec3 H = normalize(V + L);

    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));

    float metallic = clamp(uMetallic, 0.0, 1.0);
    float roughness = clamp(uRoughness, 0.0, 1.0);
    float a = max(roughness*roughness, 0.001);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float att = attenuation(lightDist, light.range);

    //if enum LightType == Spot
    //spotlight cone factor
    vec3 LtoF = normalize(fs.WorldPos - light.position); //light -> fragment
    float cutOffangle = dot(LtoF, normalize(light.direction));
    float spot = smoothstep(light.outerCutoff,
                        light.innerCutoff,
                        cutOffangle);

    // Microfacet specular
    float  D = D_GGX(NdotH, a);
    float  G = G_Smith_GGX_Schlick(NdotV, NdotL, a);
    vec3   F = F_Schlick(F0, VdotH);
    vec3  spec = (D * G) * F / max(4.0 * NdotL * NdotV, 1e-4);

    vec3 kd = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kd * albedo / PI;

    vec3 direct = (diffuse + spec) * light.intensity * NdotL * att * spot;

    return direct;
}

float ShadowFactorWS(mat4 LSpace, sampler2DShadow smap,
                     vec3 worldPos, vec3 normal, vec3 shadow_dir)
{
    vec4 clip = LSpace * vec4(worldPos, 1.0);
    vec3 proj = clip.xyz / clip.w;
    proj = proj * 0.5 + 0.5;

    if (proj.x <= 0.0 || proj.x >= 1.0 || proj.y <= 0.0 || proj.y >= 1.0 
    || proj.z <= 0.0 || proj.z >= 1.0)
        return 0.0;

    // receiver bias
    float ndotl = max(dot(normalize(normal), normalize(shadow_dir)), 0.0);
    float bias  = max(0.001, 0.005 * (1.0 - ndotl));
    float ref   = proj.z - bias;

    // hardware compare + small PCF (3x3)
    vec2 texel = 1.0 / textureSize(smap, 0);
    float lit = 0.0;
    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
        lit += texture(smap, vec3(proj.xy + vec2(x,y) * texel, ref));
    lit /= 9.0;

    return 1.0 - lit;   // shadow amount
}



void main()
{   

    vec3 color = vec3(0.0);
    vec3 albedoRGB;
    float alpha = uBaseColor.a;

    float uvScale = uUvScale;
    vec2 uv = fs.TexCoord * uvScale;
    
    // 1. Albedo
    if (uMaterial.hasDiffuseTex == 1) {
        vec3 albedo = texture(uDiffuseTex, uv).rgb;
        vec3 base    = uBaseColor.rgb;   // tint colour
        float tint   = uTint;            // 0 = no tint, 1 = full tint
        
        vec3 tinted  = albedo * base;
        albedoRGB = mix(albedo, tinted, tint);
    }else{
        albedoRGB = uBaseColor.rgb;
    }

    // 2. Normal
    vec3 N = normalize(fs.NormalWS);
    if (uMaterial.hasNormalTex == 1) {
        vec3 mapN = texture(uNormalTex, fs.TexCoord).rgb;
        mapN = normalize(mapN * 2.0 - 1.0);   // unpack [0,1] → [-1,1]
        N = normalize(fs.TBN * mapN);         // tangent space → world space
    }


    color += ambientLight * albedoRGB;

    int count = min(uLightCount, MAX_LIGHTS);
    for (int i = 0; i < count; ++i) {
        
        //default value, no shadows
        float S = 0.0;

        for (int s = 0; s < uNumShadowed; ++s) {
            //choose the same light and lightIndex
            if (uLightShadowIdx[s] == i) {
                // Choose shadowing direction for bias (dir/spot vs point)
                vec3 shadowDir = normalize(uLights[i].position - fs.WorldPos);

                S = ShadowFactorWS(uLightSpace[s], uShadowMap[s], fs.WorldPos, N, shadowDir);
                break;
            }
        }
        
        vec3 direct = shadeLight(uLights[i], N, albedoRGB);
        color += (1.0 - S) * direct;
    }



    FragColor = vec4(color, alpha);

}
