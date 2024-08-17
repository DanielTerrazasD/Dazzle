#version 430

in vec4 varyingColor;

out vec4 color;

uniform mat4 model_view_mtx;
uniform mat4 projection_mtx;

void main(void)
{
    // color = vec4(1.0, 0.0, 0.0, 1.0);
    color = varyingColor;
}