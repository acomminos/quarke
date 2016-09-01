#ifndef QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_
#define QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_

namespace quarke {
namespace pipe {

struct Mesh; // XXX(acomminos)

// Produces a G-buffer containing color, texture, depth, and normal data.
class GeometryStage {
 public:
  // Accumulates the given mesh into the geometry stage.
  void DrawMesh(const Mesh& mesh);
 private:
  GLuint fbo;
  GLuint 
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_GEOMETRY_STAGE_H_
