#version 460 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormalVector;
layout (location = 2) in vec2 vTextureCoordinates;
layout (location = 2) in vec4 vTangent;

out vec3 FragmentPosition;
out vec3 NormalVector;
out vec2 TextureCoordinates;

out vec3 LightDirection;
out vec3 ViewDirection;
// Single light source
uniform vec4 lightPosition;
uniform vec3 lightColor;

uniform mat4 m4_ModelViewProjection;
uniform mat4 m4_ModelView;
uniform mat3 m3_Normal;

void main()
{
    FragmentPosition = vec3(m4_ModelView * vec4(vPosition, 1.0));
    NormalVector = normalize(m3_Normal * vNormalVector);
    TextureCoordinates = vTextureCoordinates;

    vec3 Tangent = normalize(m3_Normal * vec3(vTangent));
    vec3 Bitangent = normalize(cross(NormalVector, Tangent)) * vTangent.w;
    mat3 TBN = transpose( mat3( Tangent, Bitangent, NormalVector ) );
    vec3 lightDirection = normalize(lightPosition.xyz - FragmentPosition);
    vec3 viewDirection = normalize(vec3(-FragmentPosition));
    LightDirection = TBN * lightDirection;
    ViewDirection = TBN * viewDirection;

    gl_Position = m4_ModelViewProjection * vec4(vPosition, 1.0);
}