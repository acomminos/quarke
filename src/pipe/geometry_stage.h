#ifndef QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_
#define QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_

#include <GLFW/glfw3.h>
#include <map>

namespace quarke {

namespace geo {
struct Material;
struct Mesh;
};  // namespace geo

namespace pipe {

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
  // Accumulates the provided mesh into the G-buffer.
  // TODO: maybe add mesh? should stages even retain geometry and rendering state?
  //       definitely add mesh to a list.
  void DrawMesh(const geo::Mesh& mesh);

  // Clears the G-buffer, overwriting all attachments with zeroes.
  void Clear();

  // Resizes the stage and clears the framebuffer.
  void SetOutputSize(int width, int height);

  // A per-material iterator over meshes to avoid excessive shader swaps.
  class MaterialIterator {
   public:
    // Returns the next mesh with the current material.
    // Returns nullptr if we've iterated over all meshes.
    virtual const geo::Mesh* Next() = 0;

    // Returns the current material being iterated over.
    virtual const geo::Material* Material() = 0;
  };

  // An iterator over a collection of meshes, per-material.
  class MeshIterator {
   public:
    // Returns the next MaterialIterator and advances the iterator, or returns
    // nullptr if we're out of elements.
    virtual MaterialIterator* Next() = 0;
  };

  // Iterates over the given mesh iterator, drawing each one per-material.
  void Render(MeshIterator& iter);

 protected:
  GLuint color_tex() const { return color_tex_; }
  GLenum color_format() const { return GL_RGBA; }

  GLuint normal_tex() const { return normal_tex_; }
  GLenum normal_format() const { return GL_RGBA; }

  GLuint depth_tex() const { return depth_tex_; }
  GLuint depth_format() const { return GL_DEPTH_COMPONENT; }

 private:
  // Constructs a vertex shader for the given material.
  // Returns 0 on failure.
  GLuint BuildVertexShader(const geo::Material& material) const;

  // Constructs a fragment shader for the given material.
  // Returns 0 on failure.
  GLuint BuildFragmentShader(const geo::Material& material) const;

  GLuint fbo_;
  GLuint color_tex_;
  GLuint normal_tex_;
  GLuint depth_tex_;

  std::map<const geo::Material*, GLuint> shader_cache_;

  int out_width_;
  int out_height_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_
