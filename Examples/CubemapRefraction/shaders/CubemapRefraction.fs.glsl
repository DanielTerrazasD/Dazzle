#version 460 core

in vec3 ReflectionDirection;
in vec3 RefractionDirection;
out vec4 FragmentColor;

layout(binding = 0) uniform samplerCube CubemapSampler;

uniform float ReflectionFactor;
uniform vec4 MaterialColor;

void main()
{
    vec3 reflectColor = texture(CubemapSampler, normalize(ReflectionDirection)).rgb;
    vec3 refractColor = texture(CubemapSampler, normalize(RefractionDirection)).rgb;

    vec3 color = mix(refractColor, reflectColor, ReflectionFactor);

    color = pow(color, vec3(1.0 / 2.2));
    FragmentColor = vec4(color, 1.0);
}
