#version 460 core

uniform vec4 Color;
out vec4 FragmentColor;

void main()
{
    FragmentColor = Color;
}