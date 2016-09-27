#ifndef QUARKE_SRC_GEO_MESH_H_
#define QUARKE_SRC_GEO_MESH_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>

namespace quarke {
namespace geo {

struct Material;

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

// A lightweight wrapper around a GL vertex data buffer to be used for sharing
// immutable buffers between meshes. It stores a VAO with attributes bound
// according to the standard mesh attribute format.
class VertexBuffer {
 public:
  const static int VS_ATTRIB_POSITION = 0; // vs index of position vec3
  const static int VS_ATTRIB_NORMAL   = 1; // vs index of normal vec3
  const static int VS_ATTRIB_TEXCOORD = 2; // vs index of texcoord vec2

  // Creates a new GL buffer owned by this VertexBuffer.
  static std::shared_ptr<VertexBuffer> Create(VertexFormat format);

  // Wraps a vertex buffer, assuming ownership.
  VertexBuffer(VertexFormat format, GLuint buffer, GLuint vao);
  VertexBuffer(const VertexBuffer& buffer) = delete;
  VertexBuffer(VertexBuffer&& buffer) = delete;
  ~VertexBuffer();

  VertexFormat format() const { return format_; }
  GLuint buffer() const { return buffer_; }
  GLuint vertex_array() const { return vao_; }
 private:

  VertexFormat format_;
  GLuint buffer_;
  GLuint vao_;
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
  // TODO: support mtl. should this output a single, or multiple meshes?
  // Returns nullptr on failure.
  static std::unique_ptr<Mesh> FromOBJ(const std::string& path);

  // Creates a new mesh using the default material and vertex data.
  Mesh(std::shared_ptr<VertexBuffer> array_buffer, GLuint num_vertices);

  // Replaces the model's current transform with the given one.
  // Coordinates are defined in world-space.
  void set_transform(const glm::mat4& transform) { transform_ = transform; }
  glm::mat4 transform() const { return transform_; }

  // TODO: remove me, and replace by generic typed maps
  glm::vec4 set_color(const glm::vec4 color) { color_ = color; }
  // Gets the mesh's inherent color, used by some material implementations.
  glm::vec4 color() const { return color_; }

  VertexBuffer& array_buffer() const { return *array_buffer_; }
  GLuint num_vertices() const { return num_vertices_; }
 private:
  // TODO. simple material ownership might not cut it.
  //Material& material_;
  glm::mat4 transform_;
  glm::vec4 color_;

  std::shared_ptr<VertexBuffer> array_buffer_;
  GLuint num_vertices_;
};

}  // namespace geo
}  // namespace quarke

#endif  // QUARKE_SRC_GEO_MESH_H_
