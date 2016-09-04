#ifndef QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_
#define QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_

#include <GLFW/glfw3.h>

namespace quarke {

namespace geo {
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
  void DrawMesh(const geo::Mesh& mesh);

  // Clears the G-buffer, overwriting all attachments with zeroes.
  void Clear();

  // Resizes the stage and clears the framebuffer.
  void SetOutputSize(int width, int height);

 protected:
  GLuint color_tex() const { return color_tex_; }
  GLenum color_format() const { return GL_RGBA; }

  GLuint normal_tex() const { return normal_tex_; }
  GLenum normal_format() const { return GL_RGBA; }

  GLuint depth_tex() const { return depth_tex_; }
  GLuint depth_format() const { return GL_DEPTH_COMPONENT; }

 private:
  GLuint fbo_;
  GLuint color_tex_;
  GLuint normal_tex_;
  GLuint depth_tex_;

  int out_width_;
  int out_height_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_
