#version 460 core

in vec3 Position;
in vec3 NormalVector;

layout(location = 0) out vec4 FragmentColor;
layout(binding = 0) uniform sampler2D RenderTextureSampler;
uniform float Weight[5];
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

vec4 Pass1()
{
    return vec4(BlinnPhongModel( Position, normalize(NormalVector) ), 1.0);
}

vec4 Pass2()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);

    vec4 sum;
    sum = texelFetch(RenderTextureSampler, pixel, 0) * Weight[0];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(0, 1)) * Weight[1];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(0, -1)) * Weight[1];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(0, 2)) * Weight[2];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(0, -2)) * Weight[2];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(0, 3)) * Weight[3];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(0, -3)) * Weight[3];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(0, 4)) * Weight[4];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(0, -4)) * Weight[4];
    return sum;
}

vec4 Pass3()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);

    vec4 sum;
    sum = texelFetch(RenderTextureSampler, pixel, 0) * Weight[0];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(1, 0)) * Weight[1];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(-1, 0)) * Weight[1];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(2, 0)) * Weight[2];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(-2, 0)) * Weight[2];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(3, 0)) * Weight[3];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(-3, 0)) * Weight[3];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(4, 0)) * Weight[4];
    sum += texelFetchOffset(RenderTextureSampler, pixel, 0, ivec2(-4, 0)) * Weight[4];
    return sum;
}

void main()
{
    if( Pass == 1 ) FragmentColor = Pass1();
    if( Pass == 2 ) FragmentColor = Pass2();
    if( Pass == 3 ) FragmentColor = Pass3();
}
