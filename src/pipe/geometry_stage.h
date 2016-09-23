#ifndef QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_
#define QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <map>
#include <memory>

namespace quarke {

namespace mat {
struct Material;
};  // namespace mat

namespace game {
struct Camera;
};  // namespace game

namespace geo {
struct Mesh;
};  // namespace geo


namespace pipe {

// A per-material iterator over meshes to avoid excessive shader swaps.
class MaterialMeshIterator {
  public:
    // Returns the next mesh with the current material.
    // Returns nullptr if we've iterated over all meshes.
    virtual const geo::Mesh* Next() = 0;

    // Returns the current material being iterated over.
    virtual mat::Material* Material() = 0;
};

// An iterator over a collection of meshes, per-material.
class MaterialIterator {
  public:
    // Returns the next MaterialIterator and advances the iterator, or returns
    // nullptr if we're out of elements.
    virtual MaterialMeshIterator* NextMaterial() = 0;
};

// Produces a G-buffer containing color, texture, depth, and normal data.
// Questions:
// - How to manage multiple materials? They might have additional per-vertex
//   and per-fragment attributes and shading.
// - I'm thinking that each material should be associated with a vs+fs.
//   That shader should use inputs and outputs specified by the geometry stage.
//   It shouldn't need to output anything more than a color buffer value, but
//   we should allow for custom vertex attribute binding.
class GeometryStage {
 public:
  static std::unique_ptr<GeometryStage> Create(int width, int height);

  GeometryStage(int width, int height, GLuint fbo, GLuint color_tex,
                GLuint normal_tex, GLuint position_tex, GLuint depth_tex);

  // Clears the G-buffer, overwriting all attachments with zeroes.
  void Clear();

  // Iterates over the given mesh iterator, drawing each one per-material.
  void Render(const game::Camera& camera, MaterialIterator& iter);

  GLuint fbo() const { return fbo_; }

  GLuint color_tex() const { return color_tex_; }
  static GLenum color_format() { return GL_RGBA; }

  GLuint normal_tex() const { return normal_tex_; }
  static GLenum normal_format() { return GL_RGBA; }

  GLuint position_tex() const { return position_tex_; }
  static GLenum position_format() { return GL_RGBA; }

  GLuint depth_tex() const { return depth_tex_; }
  static GLuint depth_format() { return GL_DEPTH_COMPONENT; }

 private:
  void SetOutputSize(int width, int height);

  // Constructs a vertex shader for the given material.
  // Returns 0 on failure.
  GLuint BuildVertexShader(const mat::Material& material) const;

  // Constructs a fragment shader for the given material.
  // Returns 0 on failure.
  GLuint BuildFragmentShader(const mat::Material& material) const;

  GLuint fbo_;
  GLuint color_tex_;
  GLuint normal_tex_;
  GLuint position_tex_;
  GLuint depth_tex_;

  // TODO: don't use raw pointers as identifiers here!
  std::map<const mat::Material*, GLuint> shader_cache_;

  int out_width_;
  int out_height_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_
