#version 460 core

flat in vec3 LightIntensity;

out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(LightIntensity, 1.0);
}
