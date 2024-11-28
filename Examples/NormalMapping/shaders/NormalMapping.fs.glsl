#version 460 core

layout(binding = 0) uniform sampler2D ColorTextureSampler;
layout(binding = 1) uniform sampler2D NormalMapSampler;

in vec3 LightDirection;
in vec3 ViewDirection;
in vec2 TextureCoordinate;
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

vec3 BlinnPhongModel(vec3 NormalFromMap)
{
    vec3 textureColor = texture(ColorTextureSampler, TextureCoordinate).rgb;

    vec3 surfacePointToLightPoint = normalize(LightDirection);
    float sDotN = max( dot( surfacePointToLightPoint, NormalFromMap ), 0.0 );

    // Calculate Ambient
    vec3 ambient = light.La * textureColor;
    // Calculate Diffuse
    vec3 diffuse = textureColor * sDotN;
    // Calculate Specular
    vec3 specular = vec3(0.0);
    if ( sDotN > 0.0 )
    {
        // surfacePointToCamera = vector from the surface point to the camera/eye.
        vec3 surfacePointToCamera = normalize(ViewDirection);
        // halfwayDirection = Halfway vector between "surfacePointToCamera" and "surfacePointToLightPoint".
        vec3 halfwayDirection = normalize(surfacePointToCamera + surfacePointToLightPoint);
        specular = material.Ks * pow( max( dot( halfwayDirection, NormalFromMap ), 0.0 ), material.Shininess );
    }

    return ambient + light.Lds * (diffuse + specular);
}

void main()
{
    // Look up the normal from the normal map
    vec3 normalFromMap = texture(NormalMapSampler, TextureCoordinate).xyz;
    // Convert range from [0, 1] to [-1, 1]
    normalFromMap.xy = 2.0 * normalFromMap.xy - 1.0;
    FragmentColor = vec4(BlinnPhongModel(normalFromMap), 1.0);
}
