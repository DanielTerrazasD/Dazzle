#version 460 core

in vec3 Position;
in vec3 NormalVector;
in vec2 TextureCoordinate;
out vec4 FragmentColor;

layout(binding = 0) uniform sampler2D BrickTextureSampler;
layout(binding = 1) uniform sampler2D MossTextureSampler;

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

vec3 BlinnPhongModel(vec3 SurfacePoint, vec3 SurfaceNormal)
{
    vec4 brickTextureColor = texture(BrickTextureSampler, TextureCoordinate);
    vec4 mossTextureColor = texture(MossTextureSampler, TextureCoordinate);
    vec3 combinedTextures = mix( brickTextureColor.rgb, mossTextureColor.rgb, mossTextureColor.a );

    vec3 surfacePointToLightPoint = normalize(vec3(light.Lp.xyz - SurfacePoint));
    float sDotN = max( dot( surfacePointToLightPoint, SurfaceNormal ), 0.0 ); // Lambertian Reflectance

    // Calculate Ambient
    vec3 ambient = light.La * combinedTextures;
    // Calculate Diffuse
    vec3 diffuse = combinedTextures * sDotN;
    // Calculate Specular
    vec3 specular = vec3(0.0);
    if ( sDotN > 0.0 )
    {
        // surfacePointToCamera = vector from the surface point to the camera/eye.
        vec3 surfacePointToCamera = normalize(-SurfacePoint.xyz);
        // halfwayDirection = Halfway vector between "surfacePointToCamera" and "surfacePointToLightPoint".
        vec3 halfwayDirection = normalize(surfacePointToCamera + surfacePointToLightPoint);
        specular = material.Ks * pow( max( dot( halfwayDirection, SurfaceNormal ), 0.0 ), material.Shininess );
    }

    return ambient + light.Lds * (diffuse + specular);
}

void main()
{
    FragmentColor = vec4(BlinnPhongModel(Position, normalize(NormalVector)), 1.0);
}
