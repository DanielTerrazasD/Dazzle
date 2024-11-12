#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 2) in vec2 VertexTextureCoordinate;

uniform mat4 MVP;

out vec3 TextureCoordinate;

void main()
{
    TextureCoordinate = vec3(VertexTextureCoordinate, 0.0);
    gl_Position = MVP * vec4(VertexPosition,1.0);
}