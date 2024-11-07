#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormalVector;

out vec3 LightIntensity;

uniform mat4 MVP;        // (mat4) Model View Projection
uniform mat4 ModelView;  // (mat4) Model View
uniform mat3 Normal;     // (mat3) Normal

uniform vec3 Kd; // (vec3) Diffuse Reflectivity
uniform vec3 Ld; // (vec3) Light Intensity
uniform vec4 Lp; // (vec4) Light Position In View Coordinates

void main(void)
{
    vec3 surfaceNormal = normalize( Normal * VertexNormalVector);
    vec4 surfacePoint = ModelView * vec4(VertexPosition,1.0);
    vec3 surfacePointToLightPoint = normalize(vec3(Lp - surfacePoint));

    LightIntensity = Ld * Kd * max( dot( surfacePointToLightPoint, surfaceNormal ), 0.0 );

    gl_Position = MVP * vec4(VertexPosition,1.0);
}