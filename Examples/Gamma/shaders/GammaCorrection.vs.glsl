#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormalVector;
layout (location = 2) in vec2 VertexTextureCoordinate;

out vec3 Position;
out vec3 NormalVector;
out vec2 TextureCoordinate;

uniform mat4 MVP;        // (mat4) Model View Projection
uniform mat4 ModelView;  // (mat4) Model View
uniform mat3 Normal;     // (mat3) Normal

void main(void)
{
    TextureCoordinate = VertexTextureCoordinate;
    Position = (ModelView * vec4(VertexPosition, 1.0)).xyz;
    NormalVector = normalize(Normal * VertexNormalVector);

    gl_Position = MVP * vec4(VertexPosition,1.0);
}