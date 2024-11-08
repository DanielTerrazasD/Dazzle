#version 460 core

in vec3 FrontFacingColor;
in vec3 BackFacingColor;

out vec4 FragmentColor;

void main()
{
    if (gl_FrontFacing)
        FragmentColor = vec4(FrontFacingColor, 1.0);
    else
        FragmentColor = vec4(BackFacingColor, 1.0);
}
