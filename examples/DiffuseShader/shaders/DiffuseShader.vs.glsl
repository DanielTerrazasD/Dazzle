#version 460 core

layout (location = 0) in vec3 vertex;
out vec4 vInterpolatedColor;

uniform mat4 mModelView;
uniform mat4 mProjection;

void main(void)
{
    vInterpolatedColor = vec4(vertex, 1.0) * 0.5 + vec4(0.5);
    gl_Position = mProjection * mModelView * vec4(vertex, 1.0);
}