#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexTextureCoordinate;

out vec3 ReflectionDirection;
out vec3 RefractionDirection;

uniform mat4 MVP;               // (mat4) Model View Projection
uniform mat4 Model;             // (mat4) Model Matrix
uniform vec3 CameraPosition;    // (vec3) Camera Position in World coordinates
uniform float RefractionIndex;

void main(void)
{
    vec3 position = vec3(Model * vec4(VertexPosition, 1.0));
    vec3 normal = vec3(Model * vec4(VertexNormal, 1.0));
    vec3 viewDirection = normalize(CameraPosition - position);

    ReflectionDirection = reflect(-viewDirection, normal);
    RefractionDirection = refract(-viewDirection, normal, RefractionIndex);
    gl_Position = MVP * vec4(VertexPosition,1.0);
}