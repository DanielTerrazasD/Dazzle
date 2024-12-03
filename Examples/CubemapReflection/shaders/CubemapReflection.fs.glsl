#version 460 core

in vec3 ReflectionDirection;
out vec4 FragmentColor;

layout(binding = 0) uniform samplerCube CubemapSampler;

uniform float ReflectionFactor;
uniform vec4 MaterialColor;

void main()
{
    vec3 color = texture(CubemapSampler, normalize(ReflectionDirection)).rgb;
    color = pow(color, vec3(1.0 / 2.2));
    FragmentColor = vec4( mix(MaterialColor.rgb, color, ReflectionFactor) , 1.0);
}
