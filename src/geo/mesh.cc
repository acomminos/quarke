#include "geo/mesh.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace quarke {
namespace geo {

/* static */
std::shared_ptr<VertexBuffer> VertexBuffer::Create(VertexFormat format) {
  GLuint buffer;
  glGenBuffers(1, &buffer);
  // TODO: should we error check here? don't think we can run out of vram.
  return std::make_shared<VertexBuffer>(format, buffer);
}

VertexBuffer::VertexBuffer(VertexFormat format, GLuint buffer)
  : format_(format), buffer_(buffer) {
}

VertexBuffer::~VertexBuffer() {
  glDeleteBuffers(1, &buffer_);
}

std::unique_ptr<Mesh> Mesh::FromOBJ(const std::string& path) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  const bool triangulate = true;
  std::string err;
  bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                                 path.c_str(), nullptr, triangulate);
  if (!err.empty()) {
    std::cerr << err << std::endl;
  }
  if (!result) {
    std::cerr << "Failed to load OBJ " << path << std::endl;
    return nullptr;
  }

  std::vector<GLfloat> data; // interleaved vertex data.
  bool hasNormals = attrib.normals.size() > 0;
  bool hasTexCoords = attrib.texcoords.size() > 0;

  // XXX: a peek into tinyobjloader would suggest that we can always expect 3
  //      component position and normal data, and 2 component texture coords.
  //      our triangulate parameter guarantees us GL_TRIANGLES as well.
  VertexFormat format = P3;
  if (hasNormals && hasTexCoords) {
    format = P3N3T2;
  } else if (hasNormals) {
    format = P3N3;
  } else if (hasTexCoords) {
    format = P3T2;
  }
  const int num_position_components = 3;
  const int num_normal_components = 3;
  const int num_uv_components = 2;
  assert(attrib.vertices.size() % num_position_components == 0);
  assert(attrib.normals.size() % num_normal_components == 0);
  assert(attrib.texcoords.size() % num_uv_components == 0);

  // TODO: Perhaps construct a element buffers ourselves later.
  //       We don't have to worry about quads.
  for (tinyobj::shape_t& shape : shapes) {
    for (tinyobj::index_t& i : shape.mesh.indices) {
      // TODO: handle per-face textures (supported by obj)
      for (int c = 0; c < num_position_components; c++) {
        data.push_back(attrib.vertices[i.vertex_index + c]);
      }

      if (hasNormals) {
        for (int c = 0; c < num_normal_components; c++) {
          data.push_back(attrib.normals[i.normal_index + c]);
        }
      } else {
        // TODO: calculate face normal here, index modulo 3?
      }

      if (hasTexCoords) {
        for (int c = 0; c < num_uv_components; c++) {
          data.push_back(attrib.texcoords[i.texcoord_index + c]);
        }
      }
    }
  }

  auto vb = VertexBuffer::Create(format);
  GLuint buffer = vb->buffer();
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat),
               (const void*) data.data(), GL_STATIC_DRAW);

  return std::make_unique<Mesh>(vb, attrib.vertices.size());
}

Mesh::Mesh(std::shared_ptr<VertexBuffer> array_buffer, GLuint num_vertices)
  : array_buffer_(array_buffer), num_vertices_(num_vertices) {
}

}  // namespace geo
}  // namespace quarke
