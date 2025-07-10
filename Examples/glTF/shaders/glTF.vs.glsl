#version 460 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormalVector;
layout (location = 2) in vec2 vTextureCoordinates;
layout (location = 3) in vec4 vTangent;

out vec3 FragmentPosition;
out vec2 TextureCoordinates;
out mat3 TBN;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat3 uNormalMatrix;     // transpose(inverse(mat3(uModelMatrix)))

uniform vec3 uCameraPosition; // Camera position in world space
uniform vec3 uLightPosition; // Light position in world space
uniform vec3 uLightColor;

void main()
{
    // Pass through texture coordinates
    TextureCoordinates = vTextureCoordinates;

    // Calculate Fragment Position in world space
    vec4 WorldPosition = uModelMatrix *  vec4(vPosition, 1.0);
    FragmentPosition = WorldPosition.xyz;

    // Transform normal and tangent to world space
    vec3 N = normalize(uNormalMatrix * vNormalVector);
    vec3 T = normalize(uNormalMatrix * vTangent.xyz);

    // // Calculate bitangent
    // vec3 B = normalize(cross(N, T));

    // Gram-Schmidt process to ensure orthogonality
    T = normalize(T - dot(T, N) * N);
    vec3 B = normalize(cross(N, T));

    // Create the TBN matrix
    TBN = mat3(T, B, N);

    // Final position
    gl_Position = uProjectionMatrix * uViewMatrix * WorldPosition;
}