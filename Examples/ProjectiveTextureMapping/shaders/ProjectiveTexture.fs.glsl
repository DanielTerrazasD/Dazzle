#version 460 core

layout (binding = 0) uniform sampler2D ProjectorSampler;

in vec3 Position;
in vec3 NormalVector;
in vec4 ProjectedTextureCoordinate;
out vec4 FragmentColor;

uniform struct Light
{
    vec3 La;            // (vec3) Ambient Intensity
    vec3 Lds;            // (vec3) Diffuse & Specular Intensity
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

void main()
{
    vec3 color = BlinnPhongModel(Position, normalize(NormalVector));

    vec3 projectedTextureColor = vec3(0.0);
    if (ProjectedTextureCoordinate.z > 0.0)
        projectedTextureColor = textureProj( ProjectorSampler, ProjectedTextureCoordinate ).rgb;

    FragmentColor = vec4(color + projectedTextureColor * 0.5, 1.0);
}