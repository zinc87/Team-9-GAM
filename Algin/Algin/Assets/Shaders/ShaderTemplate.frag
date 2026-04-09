#version 450 core
out vec4 FragColor;

//PART OF MATERIAL
layout(std140, binding=1) uniform MaterialBlock {
    vec4 baseColor;   // rgba
    vec4 mru_pad;     // metallic, roughness, uvScale, pad
} uMat;
//PART OF MATERIAL

in VS_OUT {
    vec3 WorldPos;
    vec3 NormalWS;
    vec2 TexCoord;
    mat3 TBN;
} fs;

//LIGHT
struct Light {
    vec3 position;
    float range;
    float intensity;
    vec3 direction;
    int type;
};

const int MAX_LIGHTS = 16;
uniform int   uLightCount;
uniform Light uLights[MAX_LIGHTS];
//LIGHT

//camera
uniform vec3 camPos;


//PART OF MATERIAL
//ALBEDO MAP
uniform sampler2D uAlbedoMap;
uniform bool hasAlbedoMap;

//NORMAL MAP
uniform sampler2D uNormalMap;
uniform bool hasNormalMap;
//PART OF MATERIAL

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
    float att = 1.0 / (1.0 + 0.1 * dist + 0.01 * dist * dist); // smoother falloff
    return att * step(dist, range);
}

vec3 shadePoint(Light light, vec3 normal,vec3 albedo){
    
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

    //vec3 albedo = uMat.baseColor.rgb * uMat.mru_pad.w; 
    float metallic = clamp(uMat.mru_pad.x, 0.0, 1.0);
    float roughness = clamp(uMat.mru_pad.y, 0.0, 1.0);
    float a = max(roughness*roughness, 0.001);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float att = attenuation(lightDist, light.range);

    // Microfacet specular
    float  D = D_GGX(NdotH, a);
    float  G = G_Smith_GGX_Schlick(NdotV, NdotL, a);
    vec3   F = F_Schlick(F0, VdotH);
    vec3  spec = (D * G) * F / max(4.0 * NdotL * NdotV, 1e-4);

    vec3 kd = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kd * albedo / PI;

    vec3 direct = (diffuse + spec) * light.intensity * NdotL * att;

    return direct;
}

void main() {
    
    
    vec3 color = vec3(0.0);
    vec3 albedoRGB;
    float alpha = uMat.baseColor.a;

    float uvScale = uMat.mru_pad.z;
    vec2 uv = fs.TexCoord * uvScale;

    if(hasAlbedoMap){
        vec4 texColor = texture(uAlbedoMap, uv);
        vec3 base    = uMat.baseColor.rgb;   // tint colour
        vec3 tex     = texColor.rgb;         // albedo map
        float tint   = uMat.mru_pad.w;       // 0 = no tint, 1 = full tint

        vec3 tinted  = tex * base;
        albedoRGB = mix(tex, tinted, tint);
        
    }else{
        albedoRGB = uMat.baseColor.rgb;
    }

    vec3 N = normalize(fs.NormalWS);
    if(hasNormalMap){
        vec3 mapNormal = texture(uNormalMap, uv).rgb;
        mapNormal = normalize(mapNormal*2.0 - 1.0);
        N = normalize(fs.TBN * mapNormal);
    }

    color += ambientLight * albedoRGB;

    int count = min(uLightCount, MAX_LIGHTS);
    for(int i = 0; i < count; ++i){
        vec3 direct = shadePoint(uLights[i], N, albedoRGB);
        color += direct;
    }

    FragColor = vec4(color, alpha);
     
}

