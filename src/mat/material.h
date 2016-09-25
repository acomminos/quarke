#ifndef QUARKE_SRC_MAT_MATERIAL_H_
#define QUARKE_SRC_MAT_MATERIAL_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <sstream>

namespace quarke {

namespace geo {
class Mesh;
}  // namespace geo

namespace mat {

// A homomorphism with a shader, materials encapsulate a particular
// vertex/fragment binding with logic.
//
// The geometry stage will call upon materials to generate a VS+FS.
// A material only determines the G-buffer textured output, not the lighting
// characteristics (TODO: so far).
//
// Each mesh may have custom uniforms to provide the material with, e.g. color.
// The material may access these in {Pre,Post}DrawMesh.
// FIXME: currently, we access hardcoded mesh attributes.
class Material {
 public:
  // Generates a material-specific vertex shader.
  // The material is expected to provide a "material" function for the shader.
  // This typically will set up any custom per-vertex attributes to interpolate.
  // TODO: link in the geometry stage here to reduce coupling.
  virtual void BuildVertexShader(std::ostream& vs) const = 0;
  // Generates a material-specific fragment shader.
  // The material is expected to provide the "material" function for the shader.
  // This usually only needs to set the `outColor` value for each fragment.
  // TODO: link in the geometry stage here to reduce coupling.
  virtual void BuildFragmentShader(std::ostream& fs) const = 0;

  // Binds any custom inputs built by the material.
  virtual void OnBindProgram(GLuint program) {}
  // Unbinds any custom inputs built by the material.
  virtual void OnUnbindProgram(GLuint program) {}

  // Called before drawing a mesh with this material.
  // Typically, bind mesh-specific uniforms here.
  virtual void PreDrawMesh(const geo::Mesh& mesh) {};
  // Called after drawing a mesh with this material.
  virtual void PostDrawMesh(const geo::Mesh& mesh) {};

  // Returns true if a custom VS is used for the material.
  virtual bool has_vertex_shader() const = 0;
  // Returns true if texture coordinates are used/required.
  virtual bool use_texture() const = 0;
};

}  // namespace mat
}  // namespace quarke

#endif  // QUARKE_SRC_MAT_MATERIAL_H_
