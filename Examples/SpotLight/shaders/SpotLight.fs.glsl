#version 460 core

in vec3 Position;
in vec3 NormalVector;
out vec4 FragmentColor;

uniform struct Light
{
    vec3 La;            // (vec3) Ambient Intensity
    vec3 Ld;            // (vec3) Diffuse Intensity
    vec3 Ls;            // (vec3) Specular Intensity
    vec4 Lp;            // (vec4) Light Position In View Coordinates
    vec3 Lv;            // (vec3) Light Direction Vector
    float Le;           // (float) Light Angular Attenuation Exponent
    float Lc;           // (float) Light Cutoff Angle
} light;

uniform struct Material
{
    vec3 Ka;            // (vec3) Ambient Reflectivity
    vec3 Kd;            // (vec3) Diffuse Reflectivity
    vec3 Ks;            // (vec3) Specular Reflectivity
    float Shininess;    // (float) Shininess
} material;

vec3 BlinnPhongModel(vec3 SurfacePoint, vec3 SurfaceNormal)
{
    vec3 surfacePointToLightPoint = normalize(vec3(light.Lp.xyz - SurfacePoint));
    float cosAngle = dot( -surfacePointToLightPoint, normalize(light.Lv) );
    float angle = acos(cosAngle);

    // Calculate Ambient
    vec3 ambient = light.La * material.Ka;

    // Initialize Diffuse
    vec3 diffuse = vec3(0.0);

    // Initialize Specular
    vec3 specular = vec3(0.0);

    // Initialize Angular Attenuation Scale
    float scale = 0.0;

    if (angle >= 0.0 && angle < light.Lc)
    {
        // Calculate Angular Attenuation Scale
        scale = pow(cosAngle, light.Le);

        // Calculate Lambertian Reflectance
        float sDotN = max( dot( surfacePointToLightPoint, SurfaceNormal ), 0.0 );

        // Calculate Diffuse
        diffuse = light.Ld * material.Kd * sDotN;

        // Calculate Specular
        if ( sDotN > 0.0 )
        {
            // surfacePointToCamera = vector from the surface point to the camera/eye.
            vec3 surfacePointToCamera = normalize(-SurfacePoint.xyz);
            // halfwayDirection = Halfway vector between "surfacePointToCamera" and "surfacePointToLightPoint".
            vec3 halfwayDirection = normalize(surfacePointToCamera + surfacePointToLightPoint);
            specular = light.Ls * material.Ks * pow( max( dot( halfwayDirection, SurfaceNormal ), 0.0 ), material.Shininess );
        }
    }

    return ambient + scale * (diffuse + specular);
}

void main()
{
    FragmentColor = vec4(BlinnPhongModel(Position, NormalVector), 1.0);
}
