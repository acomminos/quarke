#ifndef QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_
#define QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_

#include <GLFW/glfw3.h>

namespace quarke {

namespace geo {
struct Mesh;
};  // namespace geo

namespace pipe {

// Produces a G-buffer containing color, texture, depth, and normal data.
class GeometryStage {
 public:
  // Accumulates the provided mesh into the G-buffer.
  void DrawMesh(const geo::Mesh& mesh);

  // Clears the G-buffer, overwriting all attachments with zeroes.
  void Clear();

  GLuint color_tex() const { return color_tex_; }
  GLuint normal_tex() const { return normal_tex_; }
  GLuint depth_tex() const { return depth_tex_; }

 private:
  GLuint fbo_;
  GLuint color_tex_;
  GLuint normal_tex_;
  GLuint depth_tex_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_
