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
  GLuint buffer, vao;
  glGenBuffers(1, &buffer);
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  switch (format) {
    case VertexFormat::P3N3T2:
      glEnableVertexAttribArray(VS_ATTRIB_POSITION);
      glEnableVertexAttribArray(VS_ATTRIB_NORMAL);
      glEnableVertexAttribArray(VS_ATTRIB_TEXCOORD);

      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glVertexAttribPointer(VS_ATTRIB_POSITION, 3,
          GL_FLOAT, GL_FALSE, sizeof(GLfloat) * (3 + 3 + 2),
          (void*)0);
      glVertexAttribPointer(VS_ATTRIB_NORMAL, 3,
          GL_FLOAT, GL_FALSE, sizeof(GLfloat) * (3 + 3 + 2),
          (void*)(sizeof(GLfloat) * 3));
      glVertexAttribPointer(VS_ATTRIB_TEXCOORD, 2,
          GL_FLOAT, GL_FALSE, sizeof(GLfloat) * (3 + 3 + 2),
          (void*)(sizeof(GLfloat) * (3 + 3)));
      break;
    case VertexFormat::P3N3:
      glEnableVertexAttribArray(VS_ATTRIB_POSITION);
      glEnableVertexAttribArray(VS_ATTRIB_NORMAL);

      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glVertexAttribPointer(VS_ATTRIB_POSITION, 3,
          GL_FLOAT, GL_FALSE, sizeof(GLfloat) * (3 + 3),
          (void*)0);
      glVertexAttribPointer(VS_ATTRIB_NORMAL, 3,
          GL_FLOAT, GL_FALSE, sizeof(GLfloat) * (3 + 3),
          (void*)(sizeof(GLfloat) * 3));
      break;
    case VertexFormat::P3T2:
      glEnableVertexAttribArray(VS_ATTRIB_POSITION);
      glEnableVertexAttribArray(VS_ATTRIB_NORMAL);
      assert(false); // TODO
      break;
  }

  return std::make_shared<VertexBuffer>(format, buffer, vao);
}

VertexBuffer::VertexBuffer(VertexFormat format, GLuint buffer, GLuint vao)
  : format_(format), buffer_(buffer), vao_(vao) {
}

VertexBuffer::~VertexBuffer() {
  glDeleteBuffers(1, &buffer_);
  glDeleteVertexArrays(1, &vao_);
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

  int num_vertices = 0;
  // TODO: Perhaps construct a element buffers ourselves later.
  //       We don't have to worry about quads.
  for (tinyobj::shape_t& shape : shapes) {
    num_vertices += shape.mesh.indices.size();
    for (tinyobj::index_t& i : shape.mesh.indices) {
      // TODO: handle per-face textures (supported by obj)
      for (int c = 0; c < num_position_components; c++) {
        data.push_back(attrib.vertices[i.vertex_index * num_position_components + c]);
      }

      if (hasNormals) {
        for (int c = 0; c < num_normal_components; c++) {
          data.push_back(attrib.normals[i.normal_index * num_normal_components + c]);
        }
      } else {
        // TODO: calculate face normal here, index modulo 3?
      }

      if (hasTexCoords) {
        for (int c = 0; c < num_uv_components; c++) {
          data.push_back(attrib.texcoords[num_uv_components * i.texcoord_index + c]);
        }
      }
    }
  }

  auto vb = VertexBuffer::Create(format);
  GLuint buffer = vb->buffer();
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat),
               (const void*) data.data(), GL_STATIC_DRAW);

  return std::make_unique<Mesh>(vb, num_vertices);
}

Mesh::Mesh(std::shared_ptr<VertexBuffer> array_buffer, GLuint num_vertices)
  : array_buffer_(array_buffer), num_vertices_(num_vertices)
  , color_(glm::vec4(1.f, 1.f, 1.f, 1.f)) {
}

}  // namespace geo
}  // namespace quarke
