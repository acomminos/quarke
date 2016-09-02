#ifndef QUARKE_SRC_GEO_MESH_H_
#define QUARKE_SRC_GEO_MESH_H_

namespace quarke {
namespace geo {

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
  // Returns nullptr on failure.
  std::unique_ptr<Mesh> FromOBJ(const std::string& path);

  GLuint array_buffer() { return array_buffer_; }
  GLuint element_buffer() { return array_buffer_; }
 private:
  GLuint array_buffer_;
  GLuint element_buffer_;

  // Format of the array buffer in memory. Currently, only interleaved vertex
  // data is supported.
  //
  // Legend:
  // Pn: position
  // Nn: normal
  // Tn: uv coords
  //
  enum VertexFormat {
    P4N4T4,
    P4N4,
    P4,
  } array_buffer_format_;
};

}  // namespace geo
}  // namespace quarke

#endif  // QUARKE_SRC_GEO_MESH_H_
