#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormalVector;

flat out vec3 LightIntensity;

uniform mat4 MVP;        // (mat4) Model View Projection
uniform mat4 ModelView;  // (mat4) Model View
uniform mat3 Normal;     // (mat3) Normal

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

void main(void)
{
    vec3 surfaceNormal = normalize( Normal * VertexNormalVector);
    vec4 surfacePoint = ModelView * vec4(VertexPosition,1.0);
    vec3 surfacePointToLightPoint = normalize(vec3(light.Lp - surfacePoint));

    float sDotN = max( dot( surfacePointToLightPoint, surfaceNormal ), 0.0 );

    // Calculate Ambient
    vec3 ambient = light.La * material.Ka;
    // Calculate Diffuse
    vec3 diffuse = light.Ld * material.Kd * sDotN;
    // Calculate Specular
    vec3 specular = vec3(0.0);
    if ( sDotN > 0.0 )
    {
        // surfacePointToCamera = vector from the surface point to the camera/eye.
        vec3 surfacePointToCamera = normalize(-surfacePoint.xyz);
        // reflection = Reflected vector of the "-surfacePointToLightPoint" vector with respect to "surfaceNormal".
        vec3 reflection = reflect(-surfacePointToLightPoint, surfaceNormal);
        specular = light.Ls * material.Ks * pow( max( dot( reflection, surfacePointToCamera ), 0.0 ), material.Shininess );
    }

    LightIntensity = ambient + diffuse + specular;
    gl_Position = MVP * vec4(VertexPosition,1.0);
}