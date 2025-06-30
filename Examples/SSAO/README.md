Screen Space Ambient Occlusion (SSAO)

SSAO is a technique used to approximate ambient occlusion, a shading method that darkens surfaces
that are close to each other, mimicking the way light behaves in real life when it's blocked or occluded.

Screen Space describes that this shading technique operates in screen space (2D).
The SSAO procedure renders a depth buffer and often normals from the camera's perspective usually into a buffer.
For each pixel on the screen, a kernel of sample points in the hemisphere around that pixel (using depth and normal)
is used to estimate how much light would be blocked and darkening the pixel based on that result.
Finally a blur filter is applied to smooth noisy results.