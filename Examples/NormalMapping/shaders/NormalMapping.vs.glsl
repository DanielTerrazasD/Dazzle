#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormalVector;
layout (location = 2) in vec2 VertexTextureCoordinate;
layout (location = 3) in vec4 VertexTangent;

out vec3 LightDirection;
out vec3 ViewDirection;
out vec2 TextureCoordinate;

uniform mat4 MVP;        // (mat4) Model View Projection
uniform mat4 ModelView;  // (mat4) Model View
uniform mat3 Normal;     // (mat3) Normal

uniform struct Light
{
    vec3 La;            // (vec3) Ambient Intensity
    vec3 Lds;           // (vec3) Diffuse & Specular Intensity
    vec4 Lp;            // (vec4) Light Position In View Coordinates
} light;

void main(void)
{
    // Transform Normal and Tangent to view/camera space
    vec3 normal = normalize( Normal * VertexNormalVector);
    vec3 tangent = normalize( Normal * vec3(VertexTangent));
    // Calculate Bitangent
    vec3 bitangent = normalize( cross(normal, tangent) ) * VertexTangent.w;

    // TBN: Matrix for transformation to tangent space.
    // mat3 TBN = transpose(mat3( tangent, bitangent, normal ));
    mat3 TBN = mat3(
        tangent.x, bitangent.x, normal.x,
        tangent.y, bitangent.y, normal.y,
        tangent.z, bitangent.z, normal.z
        );

    // Transform LightDirection and ViewDirection to tangent space
    vec3 surfacePoint = vec3( ModelView * vec4(VertexPosition, 1.0));
    vec3 lightDirection = normalize(light.Lp.xyz - surfacePoint);
    vec3 viewDirection = normalize(vec3(-surfacePoint));

    LightDirection = TBN * lightDirection;
    ViewDirection = TBN * viewDirection;
    TextureCoordinate = VertexTextureCoordinate;

    gl_Position = MVP * vec4(VertexPosition,1.0);
}