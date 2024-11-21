Physical Based Rendering

It's a technique that makes use of physically-based models of light and reflection.
A PBR model is based on a microfacet surface model, energy conservation and BRDF (Bidirectional
Reflectance Distribution Function).

BRDF represents the fraction of radiance that is reflected from a surface point, given the incoming
light direction and the outgoing direction.

BRDF Cook-Torrance model is commonly used and includes diffuse and specular components.
For the diffuse component, the Lambert Reflectance model is commonly used.
The specular component is more complex, and is composed of three functions:
    - (D): Microgeometry Normal Distribution Function
    - (F): Fresnel Equation
    - (G): Geometry Function


In this example, the (D) Normal Distribution function is implemented based on physical measurements from Trowbridge and Reitz (a.k.a GGX).
The (F) Fresnel Equation for reflection is using the Schlick Approximation instead of relying on a reflection/refraction coefficients.
The (G) Geometry Function uses a combination of the GGX and Schlick-Beckmann approximation, known as Schlick-GGX.
