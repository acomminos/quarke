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
  OmniShadowStage(GLuint program, GLuint fbo, GLuint cube_texture,
                  GLsizei texture_size);

  // Constructs a cube shadow map at the given light position in world space.
  // Each shadow stage stores 6 textures.
  void BuildShadowMap(const glm::vec3 position, MaterialIterator& iter);

  // Sets the size of each dimension of the cubemapped textures.
  // `size` must be a power of two.
  void SetTextureDimension(GLsizei size);

  static GLenum depth_format() { return GL_DEPTH_COMPONENT; }
  GLuint cube_texture() { return cube_texture_; }
 private:
  // Called to render a face of the cube map.
  // Assumes:
  // - program_ is the current program.
  // - fbo_ is the bound framebuffer.
  // - cube_texture_ is bound to GL_TEXTURE_CUBE_MAP.
  void RenderFace(GLenum face, const glm::vec3 position, MaterialIterator& iter);

  static GLenum depth_internal_format() { return GL_DEPTH_COMPONENT; }

  const GLuint program_;
  const GLuint fbo_;
  GLuint uniform_transform_;

  const GLuint cube_texture_;
  GLsizei texture_size_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_OMNI_SHADOW_STAGE_H_
