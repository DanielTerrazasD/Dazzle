#version 460 core

in vec3 FragmentPosition;
in vec2 TextureCoordinates;
in mat3 TBN;

out vec4 FragmentColor;

// Material textures
layout(binding = 0) uniform sampler2D uAlbedoMap;
layout(binding = 1) uniform sampler2D uNormalMap;
layout(binding = 2) uniform sampler2D uArmMap; // AO, Roughness and Metallic Map

uniform vec3 uCameraPosition; // Camera position in world space
uniform vec3 uLightPosition; // Light position in world space
uniform vec3 uLightColor;

const float PI = 3.14159265359;

// Fresnel-Schlick approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

// Geometry function (Smith's method)
float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

void main() {
    // Sample textures
    vec3 albedo = texture(uAlbedoMap, TextureCoordinates).rgb;
    vec3 armSample = texture(uArmMap, TextureCoordinates).rgb;
    float ao = armSample.r;
    float roughness = armSample.g;
    float metallic = armSample.b;
    
    // Sample and process normal map
    vec3 normalSample = texture(uNormalMap, TextureCoordinates).rgb;
    vec3 normal = normalize(normalSample * 2.0 - 1.0);
    vec3 N = normalize(TBN * normal);
    
    // View direction (from fragment to camera) in world space
    vec3 V = normalize(uCameraPosition - FragmentPosition);
    
    // Calculate reflectance at normal incidence
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // Reflectance equation
    vec3 Lo = vec3(0.0);
    
    // Calculate per-light radiance in world space
    vec3 L = normalize(uLightPosition - FragmentPosition);
    vec3 H = normalize(V + L);
    float distance = length(uLightPosition - FragmentPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = uLightColor * attenuation;
    
    // Cook-Torrance BRDF
    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    // Add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    
    // Ambient lighting (simplified)
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;
    
    // HDR tonemapping (Reinhard)
    color = color / (color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    FragmentColor = vec4(color, 1.0);
}