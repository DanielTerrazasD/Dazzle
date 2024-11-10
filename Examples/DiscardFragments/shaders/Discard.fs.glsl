#version 460 core

uniform float scale;
uniform float factor;

in vec3 FrontFacingColor;
in vec3 BackFacingColor;
in vec2 TextureCoordinates;

out vec4 FragmentColor;

void main()
{
    bvec2 toDiscard = greaterThan( fract(TextureCoordinates * scale), vec2(factor, factor));

    if (all(toDiscard))
        discard;
    else
    {
        if (gl_FrontFacing)
            FragmentColor = vec4(FrontFacingColor, 1.0);
        else
            FragmentColor = vec4(BackFacingColor, 1.0);
    }
}
