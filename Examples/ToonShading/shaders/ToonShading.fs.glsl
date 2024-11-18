#version 460 core

in vec3 LightIntensity;
in vec3 Position;
in vec3 NormalVector;

out vec4 FragmentColor;

uniform struct Light
{
    vec3 La;            // (vec3) Ambien Intensity
    vec3 Ld;            // (vec3) Diffuse Intensity
    vec4 Lp;            // (vec4) Light Position In View Coordinates
} light;

uniform struct Material
{
    vec3 Ka;            // (vec3) Ambient Reflectivity
    vec3 Kd;            // (vec3) Diffuse Reflectivity
} material;

uniform int Levels;

vec3 ToonShade(vec3 SurfacePoint, vec3 SurfaceNormal)
{
    float scaleFactor = 1.0 / Levels;
    vec3 surfacePointToLightPoint = normalize(vec3(light.Lp.xyz - SurfacePoint));
    vec3 surfaceNormal = normalize(SurfaceNormal);

    float sDotN = max( dot( surfacePointToLightPoint, surfaceNormal ), 0.0 );

    // Calculate Ambient
    vec3 ambient = light.La * material.Ka;
    // Calculate Diffuse
    vec3 diffuse = material.Kd * floor(sDotN * Levels) * scaleFactor;
    diffuse *= light.Ld;

    return ambient + diffuse;
}

void main()
{
    FragmentColor = vec4(ToonShade(Position, NormalVector), 1.0);
}
