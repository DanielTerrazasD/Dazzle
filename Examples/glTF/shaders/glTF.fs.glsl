#version 460 core

in vec3 FragmentPosition;
in vec3 NormalVector;
in vec2 TextureCoordinates;

in vec3 ViewDirection;
in vec3 LightDirection;

out vec4 FragmentColor;

// Material textures
layout(binding = 0) uniform sampler2D albedoMap;
layout(binding = 1) uniform sampler2D normalMap;
layout(binding = 2) uniform sampler2D armMap; // AO, Roughness and Metallic Map

// Single light source
uniform vec4 lightPosition;
uniform vec3 lightColor;

const float PI = 3.14159265359;

vec3 GetNormalFromMap()
{
    // Sample the normal map
    vec3 normalFromMap = texture(normalMap, TextureCoordinates).xyz;
    // Convert range from [0, 1] to [-1, 1]
    normalFromMap.xy = 2.0 * normalFromMap.xy - 1.0;
    return normalize(normalFromMap);
}

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

// Geometry Function (Smith's method)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// Fresnel Function (Schlick's approximation)
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // Sample material properties
    vec3 albedoColor = pow(texture(albedoMap, TextureCoordinates).rgb, vec3(2.2));
    float aoValue = texture(armMap, TextureCoordinates).r;
    float roughnessValue = texture(armMap, TextureCoordinates).g;
    float metallicValue = texture(armMap, TextureCoordinates).b;

    // Use normals from model
    vec3 N = normalize(NormalVector);
    vec3 V = normalize(-FragmentPosition);
    vec3 L = normalize(lightPosition.xyz - FragmentPosition);
    vec3 H = normalize(V + L);

    // // Use normals from normal map
    // vec3 N = GetNormalFromMap();
    // vec3 V = normalize(ViewDirection);
    // vec3 L = normalize(LightDirection);
    // vec3 H = normalize(V + L);

    // Calculate reflectance at normal incidence
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedoColor, metallicValue);
    
    // Light attenuation
    float distance = length(lightPosition.xyz - FragmentPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * attenuation;
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughnessValue);
    float G = GeometrySmith(N, V, L, roughnessValue);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallicValue;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedoColor / PI + specular) * radiance * NdotL;

    // Simple ambient lighting
    vec3 ambient = vec3(0.03) * albedoColor * aoValue;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragmentColor = vec4(color, 1.0);
    // FragmentColor = vec4(V, 1.0);
}