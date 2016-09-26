#ifndef QUARKE_SRC_PIPE_OMNI_SHADOW_STAGE_H_
#define QUARKE_SRC_PIPE_OMNI_SHADOW_STAGE_H_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "pipe/geometry_stage.h" // for Material(Mesh)Iterator

namespace quarke {
namespace pipe {

// An omni-directional shadow map implementation for point lights.
// Stores cubemapped data for a single point light source.
// Roughly based on http://http.developer.nvidia.com/GPUGems/gpugems_ch12.html.
class OmniShadowStage {
 public:
  // texture_size must be a power of two.
  static std::unique_ptr<OmniShadowStage> Create(GLsizei texture_size);
  OmniShadowStage(GLuint program, GLuint fbo, GLuint textures[6],
                  GLuint cube_texture, GLsizei texture_size);

  // Constructs a cube shadow map at the given light position in world space.
  // Each shadow stage stores 6 textures.
  void BuildShadowMap(const glm::vec3 position);

  // Sets the size of each dimension of the cubemapped textures.
  // `size` must be a power of two.
  void SetTextureDimension(GLsizei size);

  static GLenum depth_format() { return GL_DEPTH_COMPONENT; }
 private:
  void RenderFace(GLenum face, const glm::vec3 position);

  static GLenum depth_internal_format() { return GL_DEPTH_COMPONENT; }

  const GLuint program_;
  const GLuint fbo_;
  GLuint uniform_transform_;

  // Depth textures mapping to each side of the cube map.
  // Ordered from +X to -Z, {X, Y, Z} alternating +/-.
  GLuint textures_[6];
  const GLuint cube_texture_;
  GLsizei texture_size_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_OMNI_SHADOW_STAGE_H_
