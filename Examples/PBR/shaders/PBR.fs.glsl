#version 460 core

in vec3 Position;
in vec3 NormalVector;
out vec4 FragmentColor;

const float PI = 3.14159265358979323846;

uniform struct Light
{
    vec4 Position;
    vec3 Intensity; // Intensity + Color
} light[3];

uniform struct Material
{
    float Roughness;
    bool Metal;
    vec3 Color;
} material;

// (D) Microgeometry Normal Distribution Function: Based on Trowbridge and Reitz a.k.a GGX
// Approximates the amount the surface's microfacets are aligned to the halfway vector,
// influenced by the roughness of the surface.
float GGXDistribution(float nDotH)
{
    float alpha2 = material.Roughness * material.Roughness * material.Roughness * material.Roughness;
    float denom = (nDotH * nDotH) * (alpha2 - 1) + 1;
    return alpha2 / (PI * denom * denom);
}

// (G) Geometry Function: Combination of GGX and Schlick-Beckmann Approximation, aka Schlick-GGX.
// Geometry Function to describe the probability that microsurfaces with a given normal
// will be visible from both the light direction (l) and the view direction (v)
float GeomSmith(float dotProd)
{
    float k = (material.Roughness + 1.0) * (material.Roughness + 1.0) / 8.0;
    float denom = dotProd * (1 - k) + k;
    return 1.0 / denom;
}

// (F) Fresnel Equation using the Schlick approximation
// Describes the ratio of surface reflection at different surface angles
vec3 SchlickFresnel(float lDotH)
{
    vec3 F0 = vec3(0.04);
    if (material.Metal)
        F0 = material.Color;

    return F0 + (1 - F0) * pow(1 - lDotH, 5);
}

// PBR: Based on a microfacet surface model, energy conservation and BRDF.
vec3 MicrofacetModel(vec3 position, vec3 n, int idx)
{
    vec3 diffuseBRDF = vec3(0.0); // Metallic Material
    if (!material.Metal)
        diffuseBRDF = material.Color;

    vec3 l = vec3(0.0);
    vec3 lightIntensity = light[idx].Intensity;

    // Directional Light
    if (light[idx].Position.w == 0.0)
    {
        l = normalize(light[idx].Position.xyz);
    }
    // Positional Light
    else
    {
        l = light[idx].Position.xyz - position; // Surface Point to Light Point
        float dist = length(l);                 // Distance from Surface Point to Light Point
        l = normalize(l);
        lightIntensity /= (dist * dist);        // Attenuation according distance
    }

    vec3 v = normalize( -position );            // Surface Point to View/Camera Point
    vec3 h = normalize( v + l );                // Halfway vector between (Surface Point to Light Point) and (Surface Point to View Point)
    float nDotH = max( dot( n, h ), 0.0 );
    float lDotH = max( dot( l, h ), 0.0 );
    float nDotL = max( dot(n, l), 0.0 );
    float nDotV = max( dot( n, v ), 0.0 );

    vec3 specularBRDF = 0.25 * GGXDistribution(nDotH) * SchlickFresnel(lDotH) * GeomSmith(nDotL) * GeomSmith(nDotV);
    return (diffuseBRDF + PI * specularBRDF) * lightIntensity * nDotL;
}

void main()
{
    vec3 summation = vec3(0.0);
    vec3 normal = normalize(NormalVector);
    for (int i = 0; i < 3; i++)
    {
        summation += MicrofacetModel(Position, normal, i);
    }

    // Gamma Correction
    summation = pow( summation, vec3(1.0/2.2) );
    FragmentColor = vec4(summation, 1.0);
}