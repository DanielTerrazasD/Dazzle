#version 460 core

layout (location = 0) in vec3 VertexPosition;

out vec3 Position;

uniform mat4 MVP;        // (mat4) Model View Projection

void main(void)
{
    Position = VertexPosition;

    gl_Position = MVP * vec4(VertexPosition,1.0);
}