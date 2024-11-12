#version 460 core

in vec3 TextureCoordinate;
out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(TextureCoordinate, 1.0);
}