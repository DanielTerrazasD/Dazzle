#version 460 core

in vec3 Position;
in vec3 NormalVector;

layout(location = 0) out vec4 FragmentColor;
layout(binding = 0) uniform sampler2D RenderTextureSampler;
uniform float EdgeThreshold;
uniform int Pass;

uniform struct Light
{
    vec3 La;            // (vec3) Ambient Intensity
    vec3 Lds;           // (vec3) Diffuse & Specular Intensity
    vec4 Lp;            // (vec4) Light Position In View Coordinates
} light;

uniform struct Material
{
    vec3 Ka;            // (vec3) Ambient Reflectivity
    vec3 Kd;            // (vec3) Diffuse Reflectivity
    vec3 Ks;            // (vec3) Specular Reflectivity
    float Shininess;    // (float) Shininess
} material;

const vec3 lum = vec3(0.2126, 0.7152, 0.0722);

vec3 BlinnPhongModel(vec3 SurfacePoint, vec3 SurfaceNormal)
{
    vec3 surfacePointToLightPoint = normalize(vec3(light.Lp.xyz - SurfacePoint));
    float sDotN = max( dot( surfacePointToLightPoint, SurfaceNormal ), 0.0 ); // Lambertian Reflectance

    // Calculate Ambient
    vec3 ambient = light.La * material.Ka;
    // Calculate Diffuse
    vec3 diffuse = material.Kd * sDotN;
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

float Luminance(vec3 color)
{
    return dot(lum, color);
}

vec4 Pass1()
{
    return vec4(BlinnPhongModel( Position, normalize(NormalVector) ), 1.0);
}

vec4 Pass2()
{
    ivec2 pix = ivec2(gl_FragCoord.xy);

    float s00 = Luminance(texelFetchOffset(RenderTextureSampler, pix, 0, ivec2(-1,1)).rgb);
    float s10 = Luminance(texelFetchOffset(RenderTextureSampler, pix, 0, ivec2(-1,0)).rgb);
    float s20 = Luminance(texelFetchOffset(RenderTextureSampler, pix, 0, ivec2(-1,-1)).rgb);
    float s01 = Luminance(texelFetchOffset(RenderTextureSampler, pix, 0, ivec2(0,1)).rgb);
    float s21 = Luminance(texelFetchOffset(RenderTextureSampler, pix, 0, ivec2(0,-1)).rgb);
    float s02 = Luminance(texelFetchOffset(RenderTextureSampler, pix, 0, ivec2(1,1)).rgb);
    float s12 = Luminance(texelFetchOffset(RenderTextureSampler, pix, 0, ivec2(1,0)).rgb);
    float s22 = Luminance(texelFetchOffset(RenderTextureSampler, pix, 0, ivec2(1,-1)).rgb);

    float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
    float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);

    float g = sx * sx + sy * sy;

    if( g > EdgeThreshold )
        return vec4(1.0);
    else
        return vec4(0.0,0.0,0.0,1.0);
}

void main()
{
    if( Pass == 1 ) FragmentColor = Pass1();
    if( Pass == 2 ) FragmentColor = Pass2();
}
