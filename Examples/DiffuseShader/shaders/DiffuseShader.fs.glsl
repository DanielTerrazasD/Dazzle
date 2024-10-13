#version 460 core

in vec4 vInterpolatedColor;
out vec4 vColor;

void main(void)
{
    vColor = vInterpolatedColor;
}