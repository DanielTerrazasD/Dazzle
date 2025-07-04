#version 460 core

in vec3 Position;
in vec3 NormalVector;

layout(location = 0) out vec4 FragmentColor;

struct Light
{
    vec3 mIntensity;            // (vec3) Ambient Intensity
    vec4 mPosition;             // (vec4) Light Position In View Coordinates
};

struct Material
{
    vec4 mAmbient;              // (vec3) Ambient Reflectivity
    vec4 mDiffuse;              // (vec3) Diffuse Reflectivity
};

uniform Light light; // Lights source data
uniform Material material; // Material properties



#define MAX_FRAGMENTS 75
subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;
uniform uint MaxNodes;

struct NodeType
{
    vec4 mColor;
    float mDepth;
    uint mNext;
};

layout( binding = 0, r32ui ) uniform uimage2D HeadPointers;
layout( binding = 0, offset = 0 ) uniform atomic_uint NextNodeCounter;
layout( binding = 0, std430 ) buffer LinkedLists { NodeType mNodes[]; };


vec3 Diffuse()
{
    vec3 surfacePointToLightPoint = normalize(light.mPosition.xyz - Position);
    vec3 surfaceNormal = normalize(NormalVector);

    return light.mIntensity * (material.mAmbient.rgb + material.mDiffuse.rgb * max( dot(surfacePointToLightPoint, surfaceNormal), 0.0 ));
}

subroutine(RenderPassType)
void Pass1()
{
    uint nodeIndex = atomicCounterIncrement(NextNodeCounter);

    if (nodeIndex < MaxNodes)
    {
        uint previousHead = imageAtomicExchange(HeadPointers, ivec2(gl_FragCoord.xy), nodeIndex);

        mNodes[nodeIndex].mColor = vec4(Diffuse(), material.mDiffuse.a);
        mNodes[nodeIndex].mDepth = gl_FragCoord.z;
        mNodes[nodeIndex].mNext = previousHead;
    }
}

subroutine(RenderPassType)
void Pass2()
{
    NodeType fragments[MAX_FRAGMENTS];
    int counter = 0;

    uint n = imageLoad(HeadPointers, ivec2(gl_FragCoord.xy)).r;

    while (n != 0xFFFFFFFF && counter < MAX_FRAGMENTS)
    {
        fragments[counter] = mNodes[n];
        n = fragments[counter].mNext;
        counter++;
    }

    for (uint i = 1; i < counter; i++)
    {
        NodeType toInsert = fragments[i];
        uint j = i;
        while (j > 0 && toInsert.mDepth > fragments[j - 1].mDepth)
        {
            fragments[j] = fragments[j - 1];
            j--;
        }
        fragments[j] = toInsert;
    }

    vec4 color = vec4(0.5, 0.5, 0.5, 1.0);
    for (int i = 0; i < counter; i++)
    {
        color = mix(color, fragments[i].mColor, fragments[i].mColor.a);
    }

    FragmentColor = color;
}

void main()
{
    RenderPass();
}
