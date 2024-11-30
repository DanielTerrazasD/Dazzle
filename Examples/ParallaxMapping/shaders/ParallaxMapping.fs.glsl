#version 460 core

layout(binding = 0) uniform sampler2D ColorTextureSampler;
layout(binding = 1) uniform sampler2D NormalMapSampler;
layout(binding = 2) uniform sampler2D HeightMapSampler;

in vec3 LightDirection;
in vec3 ViewDirection;
in vec2 TextureCoordinates;
out vec4 FragmentColor;

uniform struct Light
{
    vec3 La;            // (vec3) Ambient Intensity
    vec3 Lds;           // (vec3) Diffuse & Specular Intensity
    vec4 Lp;            // (vec4) Light Position In View Coordinates
} light;

uniform struct Material
{
    vec3 Ks;            // (vec3) Specular Reflectivity
    float Shininess;    // (float) Shininess
} material;

uniform float BumpFactor;
uniform int Steps;

const int PARALLAX = 0;
const int STEEP_PARALLAX = 1;
uniform int MappingMode; // 0 = Parallax, 1 = Steep Parallax

vec2 FindOffset(vec3 viewDirection, vec2 TexCoord, out float height)
{
    if (MappingMode == PARALLAX) // Parallax
    {
        // Height value from Height Map
        float heightFromMap = texture(HeightMapSampler, TexCoord).r;
        height = 1 - heightFromMap;

        // Calculate texture delta and apply offset to texture coordinates
        vec2 delta = viewDirection.xy * height * BumpFactor / viewDirection.z;
        vec2 textureCoord = TexCoord.xy - delta;
        return textureCoord;
    }
    else // Steep Parallax
    {
        const int nSteps = int( mix(Steps, 10, abs(viewDirection.z) ) );
        // Value of the differential of each height delta
        float heightStep = 1.0 / nSteps;
        // Value of the differential of each Texture Coordinate delta
        vec2 deltaTexCoord = (viewDirection.xy * BumpFactor) / (nSteps * viewDirection.z);
        // Initial Height value
        float ht = 1.0;
        // Initial Texture Coordinate value
        vec2 tc = TexCoord;
        // Initializing height
        height = texture(HeightMapSampler, tc).r;
        while (height < ht && ht > 0.0)
        {
            ht -= heightStep;
            tc -= deltaTexCoord;
            height = texture(HeightMapSampler, tc).r;
        }
        return tc;
    }
}

bool IsOccluded(vec3 lightDirection, vec2 TexCoord, float height)
{
    const int nShadowSteps = int( mix(Steps, 10, abs(lightDirection.z) ) );
    // Value of the differential of each height delta
    float heightStep = 1.0 / nShadowSteps;
    // Value of the differential of each Texture Coordinate delta
    vec2 deltaTexCoord = (lightDirection.xy * BumpFactor) / (nShadowSteps * lightDirection.z);
    // Initial Height value
    float ht = height + heightStep * 0.1;
    // Initial Texture Coordinate value
    vec2 tc = TexCoord;
    while (height < ht && ht < 1.0)
    {
        ht += heightStep;
        tc += deltaTexCoord;
        height = texture(HeightMapSampler, tc).r;
    }

    return ht < 1.0;
}

vec3 BlinnPhongModel()
{
    // Normalize Light and View directions (in tangent space)
    vec3 surfacePointToCamera = normalize(ViewDirection);
    vec3 surfacePointToLightPoint = normalize(LightDirection);

    // Calculate Texture Coordinates offset and height
    float height = 1.0;
    vec2 textureCoord = FindOffset(surfacePointToCamera, TextureCoordinates, height);

    // Color from Diffuse Texture using modified Texture Coordinates
    vec3 textureColor = texture(ColorTextureSampler, textureCoord).rgb;

    // Normal vector from Normal Map using modified Texture Coordinates
    vec3 normalFromMap = texture(NormalMapSampler, textureCoord).xyz;
    normalFromMap.xy = 2.0 * normalFromMap.xy - 1.0; // Convert range from [0, 1] to [-1, 1]
    normalFromMap = normalize(normalFromMap);

    // Lambertian Reflectance
    float sDotN = max( dot( surfacePointToLightPoint, normalFromMap ), 0.0 );

    // Calculate Ambient
    vec3 ambient = light.La * textureColor;
    // Calculate Diffuse
    vec3 diffuse = textureColor * sDotN;
    // Calculate Specular
    vec3 specular = vec3(0.0);
    // Only when using Steep Parallax: Check if fragment is occluded by another surface from the height map
    bool isOccluded = (MappingMode == STEEP_PARALLAX) && IsOccluded(surfacePointToLightPoint, textureCoord, height);
    if (sDotN <= 0.0 || isOccluded)
    {
        // Light is not facing the surface or fragment is occluded. So, diffuse is zero
        diffuse = vec3(0.0);
    }
    else
    {
        // halfwayDirection = Halfway vector between "surfacePointToCamera" and "surfacePointToLightPoint".
        vec3 halfwayDirection = normalize(surfacePointToCamera + surfacePointToLightPoint);
        specular = material.Ks * pow( max( dot( halfwayDirection, normalFromMap ), 0.0 ), material.Shininess );
    }

    return ambient + light.Lds * (diffuse + specular);
}

void main()
{
    vec3 color = BlinnPhongModel();
    color = pow(color, vec3(1.0/2.2));
    FragmentColor = vec4(color, 1.0);
}
