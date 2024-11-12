#version 460 core

in vec3 Normal;
out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4((Normal * 0.5) + 0.5, 1.0);
}