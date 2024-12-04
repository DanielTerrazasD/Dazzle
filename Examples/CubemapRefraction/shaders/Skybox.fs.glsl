#version 460 core

in vec3 Position;
out vec4 FragmentColor;

layout(binding = 1) uniform samplerCube SkyboxSampler;

void main()
{
    vec3 color = texture(SkyboxSampler, normalize(Position)).rgb;
    color = pow(color, vec3(1.0 / 2.2));
    FragmentColor = vec4(color, 1.0);
}
