#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormalVector;
layout (location = 2) in vec2 VertexTextureCoordinates;

out vec3 FrontFacingColor;
out vec3 BackFacingColor;
out vec2 TextureCoordinates;

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

vec3 PhongModel(vec3 SurfacePoint, vec3 SurfaceNormal)
{
    vec3 surfacePointToLightPoint = normalize(vec3(light.Lp.xyz - SurfacePoint));
    float sDotN = max( dot( surfacePointToLightPoint, SurfaceNormal ), 0.0 );

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
        // reflection = Reflected vector of the "-surfacePointToLightPoint" vector with respect to "surfaceNormal".
        vec3 reflection = reflect(-surfacePointToLightPoint, SurfaceNormal);
        specular = light.Ls * material.Ks * pow( max( dot( reflection, surfacePointToCamera ), 0.0 ), material.Shininess );
    }

    return ambient + diffuse + specular;
}

void main(void)
{
    vec3 surfaceNormal = normalize( Normal * VertexNormalVector);
    vec3 surfacePoint = (ModelView * vec4(VertexPosition,1.0)).xyz;

    FrontFacingColor = PhongModel(surfacePoint, surfaceNormal);
    BackFacingColor = PhongModel(surfacePoint, -surfaceNormal);
    TextureCoordinates = VertexTextureCoordinates;

    gl_Position = MVP * vec4(VertexPosition,1.0);
}