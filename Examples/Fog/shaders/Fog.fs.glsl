#version 460 core

in vec3 Position;
in vec3 NormalVector;

out vec4 FragmentColor;

uniform struct Light
{
    vec3 La;            // (vec3) Ambien Intensity
    vec3 Ld;            // (vec3) Diffuse Intensity
    vec3 Ls;            // (vec3) Specular Intensity
    vec4 Lp;            // (vec4) Light Position In View Coordinates
} light;

uniform struct Material
{
    vec3 Ka;            // (vec3) Ambient Reflectivity
    vec3 Kd;            // (vec3) Diffuse Reflectivity
    vec3 Ks;            // (vec3) Specular Reflectivity
    float Shininess;    // (float) Shininess
} material;

uniform struct Fog
{
    float Density;
    float MaxDistance;
    float MinDistance;
    vec3 Color;
    int Model; // 0 = Linear; 1 = Exponential; 2 = Quadratic
} fog;


vec3 BlinnPhongModel(vec3 SurfacePoint, vec3 SurfaceNormal)
{
    vec3 surfacePointToLightPoint = normalize(vec3(light.Lp.xyz - SurfacePoint));
    float sDotN = max( dot( surfacePointToLightPoint, SurfaceNormal ), 0.0 ); // Lambertian Reflectance

    // Calculate Ambient
    vec3 ambient = light.La * material.Ka;
    // Calculate Diffuse
    vec3 diffuse = light.Ld * material.Kd * sDotN;
    // Calculate Specular
    vec3 specular = vec3(0.0);
    if ( sDotN > 0.0 )
    {
        // surfacePointToCamera = vector from the surface point to the camera/eye.
        vec3 surfacePointToCamera = normalize(-SurfacePoint.xyz);
        // halfwayDirection = Halfway vector between "surfacePointToCamera" and "surfacePointToLightPoint".
        vec3 halfwayDirection = normalize(surfacePointToCamera + surfacePointToLightPoint);
        specular = light.Ls * material.Ks * pow( max( dot( halfwayDirection, SurfaceNormal ), 0.0 ), material.Shininess );
    }

    return ambient + diffuse + specular;
}

void main()
{
    float distanceToSurface = length(Position);

    float fogFactor = 0.0;
    if (fog.Model == 2)
        fogFactor = exp(-pow(distanceToSurface * fog.Density, 2.0));
    else if (fog.Model == 1)
        fogFactor = exp(-distanceToSurface * fog.Density);
    else
        fogFactor = (fog.MaxDistance - distanceToSurface) / (fog.MaxDistance - fog.MinDistance);

    fogFactor = clamp(fogFactor, 0.0, 1.0);
    vec3 color = BlinnPhongModel(Position, normalize(NormalVector));
    color = mix(fog.Color, color, fogFactor);
    FragmentColor = vec4(color, 1.0);
}
