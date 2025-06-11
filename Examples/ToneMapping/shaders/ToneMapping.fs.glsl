#version 460 core

in vec3 Position;
in vec3 NormalVector;
in vec2 TextureCoordinates;

layout(location = 0) out vec4 FragmentColor;
layout(location = 1) out vec3 HDRColor;

layout(binding = 0) uniform sampler2D HDRTexture;

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
};

uniform Light lights[3]; // Lights source data
uniform Material material; // Material properties

uniform int Pass; // Current pass number, 1 or 2
uniform float AverageLuminance; // Average Luminance of the scene
uniform bool EnableToneMapping; // Enable or disable tone mapping
uniform float Exposure;         // Exposure value for tone mapping
// Reference brightness level that is considered “white” in the final image.
uniform float WhitePoint;      // White point for tone mapping

// Conversion matrix for RGB to XYZ color space
uniform mat3 RGB2XYZ = mat3(
    0.4124564, 0.2126729, 0.0193339,
    0.3575761, 0.7151522, 0.1191920,
    0.1804375, 0.0721750, 0.9503041
);

// Conversion matrix for XYZ to RGB color space
uniform mat3 XYZ2RGB = mat3(
    3.2404542, -0.9692660, 0.0556434,
    -1.5371385, 1.8760108, -0.2040259,
    -0.4985314, 0.0415560, 1.0572252
);

vec3 BlinnPhongModel(vec3 SurfacePoint, vec3 SurfaceNormal, int idx)
{
    vec3 surfacePointToLightPoint = normalize(vec3(lights[idx].mPosition.xyz - SurfacePoint));
    float sDotN = max( dot( surfacePointToLightPoint, SurfaceNormal ), 0.0 ); // Lambertian Reflectance

    // Calculate Ambient
    vec3 ambient = lights[idx].mAmbient * material.mAmbient;
    // Calculate Diffuse
    vec3 diffuse = material.mDiffuse * sDotN;
    // Calculate Specular
    vec3 specular = vec3(0.0);
    if ( sDotN > 0.0 )
    {
        // surfacePointToCamera = vector from the surface point to the camera/eye.
        vec3 surfacePointToCamera = normalize(-SurfacePoint.xyz);
        // halfwayDirection = Halfway vector between "surfacePointToCamera" and "surfacePointToLightPoint".
        vec3 halfwayDirection = normalize(surfacePointToCamera + surfacePointToLightPoint);
        specular = material.mSpecular * pow( max( dot( halfwayDirection, SurfaceNormal ), 0.0 ), material.mShininess );
    }

    return ambient + lights[idx].mDiffuseSpecular * (diffuse + specular);
}

void Pass1()
{
    vec3 Normal = normalize(NormalVector);

    // Compute shading and store result in a high-resolution framebuffer.
    HDRColor = vec3(0.0);
    for (int i = 0; i < 3; ++i)
    {
        HDRColor += BlinnPhongModel(Position, Normal, i);
    }
}

void Pass2()
{
    vec4 color = texture(HDRTexture, TextureCoordinates);

    // Convert the color from RGB to XYZ color space
    vec3 xyzColor = RGB2XYZ * color.rgb;
    float xyzSum = xyzColor.x + xyzColor.y + xyzColor.z;

    // Convert XYZ to xyY color space
    vec3 xyYColor = vec3(xyzColor.x / xyzSum, xyzColor.y / xyzSum, xyzColor.y);

    // Apply the tone mapping operator
    // Scale luminance by exposure and average luminance
    float Luminance = Exposure * xyYColor.z / AverageLuminance;
    Luminance = (Luminance * (1 + Luminance / (WhitePoint * WhitePoint))) / (1 + Luminance);

    // Convert back to XYZ color space using the new luminance
    xyzColor.x = (Luminance * xyYColor.x) / xyYColor.y;
    xyzColor.y = Luminance;
    xyzColor.z = (Luminance * (1 - xyYColor.x - xyYColor.y)) / xyYColor.y;

    if (EnableToneMapping)
        FragmentColor = vec4(XYZ2RGB * xyzColor, 1.0);
    else
        FragmentColor = vec4(color);
}

void main()
{
    if( Pass == 1 ) Pass1();
    else if( Pass == 2 ) Pass2();
}
