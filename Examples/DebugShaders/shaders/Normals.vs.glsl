#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormalVector;

uniform mat4 MVP;

out vec3 Normal;

void main()
{
    Normal = VertexNormalVector;
    gl_Position = MVP * vec4(VertexPosition,1.0);
}