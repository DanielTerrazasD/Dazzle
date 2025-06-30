#version 460 core

in vec3 Position;
in vec3 NormalVector;
in vec2 TextureCoordinates;

layout(location = 0) out vec4 FragmentColor;
layout(location = 1) out vec3 PositionData;
layout(location = 2) out vec3 NormalData;
layout(location = 3) out vec3 ColorData;
layout(location = 4) out float AOData;

layout(binding = 0) uniform sampler2D PositionTexture;
layout(binding = 1) uniform sampler2D NormalTexture;
layout(binding = 2) uniform sampler2D ColorTexture;
layout(binding = 3) uniform sampler2D AOTexture;
layout(binding = 4) uniform sampler2D RandomTexture;
layout(binding = 5) uniform sampler2D DiffuseTexture;

struct Light
{
    vec3 mAmbient;              // (vec3) Ambient Intensity
    vec3 mDiffuseSpecular;      // (vec3) Diffuse & Specular Intensity
    vec4 mPosition;             // (vec4) Light Position In View Coordinates
};

struct Material
{
    vec3 mAmbient;              // (vec3) Ambient Reflectivity
    vec3 mDiffuse;              // (vec3) Diffuse Reflectivity
    vec3 mSpecular;             // (vec3) Specular Reflectivity
    float mShininess;           // (float) Shininess
    bool mUseTexture;           // (bool) Whether to use texture or not
};

const int kKernelSize = 64; // Size of the SSAO sample kernel
const vec2 RandomScale = vec2(1280.0 / 4.0, 720.0 / 4.0); // Scale for random texture coordinates, directly related to the resolution of the screen/buffer

uniform Light light; // Lights source data
uniform Material material; // Material properties
uniform int Pass; // Current pass number
uniform mat4 ProjectionMatrix;
uniform vec3 SampleKernel[kKernelSize]; // Sample kernel for SSAO
uniform float Radius = 0.55; // Radius for SSAO

vec3 AmbientAndDiffuse(vec3 SurfacePoint, vec3 SurfaceNormal, vec3 DiffuseColor, float AO)
{
    AO = pow(AO, 4.0);
    vec3 ambient = light.mAmbient * DiffuseColor * AO;
    vec3 surfacePointToLightPoint = normalize(vec3(light.mPosition.xyz - SurfacePoint));
    float sDotN = max(dot(surfacePointToLightPoint, SurfaceNormal), 0.0); // Lambertian Reflectance
    return ambient + light.mDiffuseSpecular * (DiffuseColor * sDotN);
} 

void Pass1()
{
    // Store position, normal, and color in the output buffers
    PositionData = Position;
    NormalData = normalize(NormalVector);
    if (material.mUseTexture)
        ColorData = pow(texture(DiffuseTexture, TextureCoordinates).rgb, vec3(2.2)); // Apply gamma correction
    else
        ColorData = material.mDiffuse;
}

void Pass2()
{
    vec3 randomDirection = normalize( texture(RandomTexture, TextureCoordinates * RandomScale).xyz );
    vec3 normal = normalize( texture(NormalTexture, TextureCoordinates).xyz );
    vec3 bitangent = cross(normal, randomDirection);
    if (length(bitangent) < 0.0001)
        bitangent = cross(normal, vec3(0.0, 0.0, 1.0)); // Fallback to a default direction
    bitangent = normalize(bitangent);
    vec3 tangent = cross(bitangent, normal);
    mat3 TBN = mat3(tangent, bitangent, normal); // Tangent-Bitangent-Normal matrix

    float occlusionSum = 0.0;
    vec3 cameraPosition = texture(PositionTexture, TextureCoordinates).xyz;
    for (int i = 0; i < kKernelSize; ++i)
    {
        vec3 samplePosition = cameraPosition + Radius * (TBN * SampleKernel[i]);
        
        // Project point
        vec4 point = ProjectionMatrix * vec4(samplePosition, 1.0);
        point *= 1.0 / point.w; // Perspective divide
        point.xyz = point.xyz * 0.5 + 0.5; // Convert to [0, 1] range

        // Access camera space z-coordinate at that point
        float surfaceDepth = texture(PositionTexture, point.xy).z;
        float depthDistance = surfaceDepth - cameraPosition.z;

        if (depthDistance >= 0.0 && depthDistance <= Radius && surfaceDepth > samplePosition.z)
        {
            // Calculate occlusion based on the depth difference
            occlusionSum += 1.0;
        }

        float occlusionFactor = occlusionSum / float(kKernelSize);
        AOData = 1.0 - occlusionFactor; // Invert to get the ambient occlusion factor
    }
}

void Pass3()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    float sum = 0.0;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            sum += texelFetchOffset(AOTexture, pixel, 0, ivec2(x, y)).r;
        }
    }

    float AO = sum * (1.0 / 9.0); // Average the surrounding pixels
    AOData = AO; // Store the ambient occlusion value
}

void Pass4()
{
    // Retrieve position, normal, and color data
    vec3 position = texture(PositionTexture, TextureCoordinates).xyz;
    vec3 normal = texture(NormalTexture, TextureCoordinates).xyz;
    vec3 diffuseColor = texture(ColorTexture, TextureCoordinates).rgb;
    float AOValue = texture(AOTexture, TextureCoordinates).r;

    vec3 color = AmbientAndDiffuse(position, normal, diffuseColor, AOValue);
    color = pow(color, vec3(1.0 / 2.2)); // Apply gamma correction
    FragmentColor = vec4(color, 1.0); // Output the final color
}

void main()
{
    if( Pass == 1 ) Pass1();
    else if( Pass == 2 ) Pass2();
    else if( Pass == 3 ) Pass3();
    else if( Pass == 4 ) Pass4();
}
