Normal Mapping

Using a normal map that essentially contains normals in tangent space to enhance the details for
a 3D surface by simulating higher resolution details on low-polygon models.

Essentially a normal map is a texture where RGB channels represent the X, Y, and Z components of a normal vector.
These normals replace or perturb the original surface normal during light calculations.
These perturbed normals influence lighting such that it appears like the surface features cracks, wrinkles, etc. on a low-polygon mesh.
 