#version 430

layout (location=0) in vec3 position;

uniform mat4 model_view_mtx;
uniform mat4 projection_mtx;

out vec4 varyingColor;

void main(void)
{
    gl_Position = projection_mtx * model_view_mtx * vec4(position, 1.0);
    varyingColor = vec4(position, 1.0) * 0.5 + vec4(0.5, 0.5, 0.5, 0.5);
}