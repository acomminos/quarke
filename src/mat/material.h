#ifndef QUARKE_SRC_MAT_MATERIAL_H_
#define QUARKE_SRC_MAT_MATERIAL_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <sstream>

namespace quarke {
namespace mat {

// A homomorphism with a shader, materials encapsulate a particular
// vertex/fragment binding with logic.
//
// The geometry stage will call upon materials to generate a VS+FS.
// A material only determines the G-buffer textured output, not the lighting
// characteristics (TODO: so far).
//
// XXX: stub
class Material {
 public:
  // Generates a material-specific vertex shader.
  // The material is expected to provide a "material" function for the shader.
  // This typically will set up any custom per-vertex attributes to interpolate.
  // TODO: link in the geometry stage here to reduce coupling.
  virtual void BuildVertexShader(std::ostream& vs) const = 0;
  // Generates a material-specific fragment shader.
  // The material is expected to provide the "main" function for the shader, as
  // well as populate all outputs of the geometry stage pipeline.
  // TODO: link in the geometry stage here to reduce coupling.
  virtual void BuildFragmentShader(std::ostream& fs) const = 0;

  // Returns true if a custom VS is used for the material.
  virtual bool has_vertex_shader() const = 0;
  // Returns true if texture coordinates are used/required.
  virtual bool use_texture() const = 0;

  // Binds any custom inputs built by the material.
  virtual void OnBindProgram(GLuint program) {}
  // Unbinds any custom inputs built by the material.
  virtual void OnUnbindProgram(GLuint program) {}
};

}  // namespace mat
}  // namespace quarke

#endif  // QUARKE_SRC_MAT_MATERIAL_H_
