#version 460 core

in vec3 LightIntensity;

out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(LightIntensity, 1.0);
}
