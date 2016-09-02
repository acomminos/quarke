#ifndef QUARKE_SRC_GEO_MESH_H_
#define QUARKE_SRC_GEO_MESH_H_

#include <GLFW/glfw3.h>
#include <memory>

namespace quarke {
namespace geo {

// Format of the array buffer in memory. Currently, only interleaved vertex
// data is supported with floating point values.
//
// Legend:
// Pn: position
// Nn: normal
// Tn: uv coords
//
// where n is the number of 32-bit floating point components.
enum VertexFormat {
  P3N3T2,
  P3N3,
  P3T2,
  P3,
};

// A mesh is simply an aggregation of triangle faces.
// XXX: idea
// - split rendering passes batched by texture
// - execute glDrawX per-texture, not per-face
// - can we do this? why not sort faces by texture? one VBO/tex?
// - actually, why not just one VBO period. however, the faces in the VBO must
//   be continuous with respect to the active texture- we can't have sparse
//   binding of vertex data.
// One texture per mesh would be simplest, for now. We can do most of what we want with uv-mapping.
class Mesh {
 public:
  // Loads a mesh given a path to an obj file.
  // TODO: support mtl.
  // Returns nullptr on failure.
  std::unique_ptr<Mesh> FromOBJ(const std::string& path);

  GLuint array_buffer() { return array_buffer_; }
  VertexFormat array_buffer_format() { return array_buffer_format_; }
  GLuint element_buffer() { return element_buffer_; }
 private:
  GLuint array_buffer_;
  VertexFormat array_buffer_format_;
  GLuint element_buffer_;
};

}  // namespace geo
}  // namespace quarke

#endif  // QUARKE_SRC_GEO_MESH_H_
