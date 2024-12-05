#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormalVector;

out vec3 Position;
out vec3 NormalVector;
out vec4 ProjectedTextureCoordinate;

uniform mat4 MVP;       // (mat4) Model View Projection
uniform mat4 Model;     // (mat4) Model
uniform mat4 ModelView; // (mat4) Model View
uniform mat3 Normal;    // (mat3) Normal

uniform mat4 ProjectorMatrix;

void main(void)
{
    vec4 vertexPosition = vec4(VertexPosition, 1.0);

    Position = (ModelView * vertexPosition).xyz;
    NormalVector = normalize( Normal * VertexNormalVector);
    ProjectedTextureCoordinate = ProjectorMatrix * (Model * vertexPosition);

    gl_Position = MVP * vertexPosition;
}