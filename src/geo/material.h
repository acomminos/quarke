#ifndef QUARKE_SRC_GEO_MATERIAL_H_
#define QUARKE_SRC_GEO_MATERIAL_H_

#include <GLFW/glfw3.h>
#include <sstream>

namespace quarke {
namespace geo {

// A homomorphism with a shader, materials encapsulate a particular
// vertex/fragment binding with logic.
//
// The geometry stage will call upon materials to generate a VS+FS.
//
// XXX: stub
class Material {
 public:
  // Adds material-specific input to a vertex shader.
  // The material is expected to provide the "main" function for the shader, as
  // well as populate all outputs of the geometry stage pipeline.
  // TODO: link in the geometry stage here to reduce coupling.
  virtual void BuildVertexShader(std::ostream& vs);
  // Adds material-specific input to a frragment shader.
  // The material is expected to provide the "main" function for the shader, as
  // well as populate all outputs of the geometry stage pipeline.
  // TODO: link in the geometry stage here to reduce coupling.
  virtual void BuildFragmentShader(std::ostream& fs);

  // Binds any custom inputs built by the material.
  virtual void OnBindProgram(GLuint program);
  // Unbinds any custom inputs built by the material.
  virtual void OnUnbindProgram(GLuint program);
};

}  // namespace geo
}  // namespace quarke

#endif  // QUARKE_SRC_GEO_MATERIAL_H_
