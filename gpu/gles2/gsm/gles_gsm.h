// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_GLES2_GSM_GLES_GSM_H_
#define GPU_GLES2_GSM_GLES_GSM_H_

#include "gpu/gles2/context/gles_context.h"
#include "gpu/gles2/draw/quad_drawable.h"
#include "gpu/gles2/gsm/state_stacks.h"
#include "gpu/gles2/shader/shader_manager.h"

namespace gpu {

class GlobalStateManager;
class QuadIndexBuffer;

extern GlobalStateManager GSM;

class GlobalStateManager final {
 public:
  struct GLShaderWare {
    BaseShader base;
  };

  GlobalStateManager() = default;

  GlobalStateManager(const GlobalStateManager&) = delete;
  GlobalStateManager& operator=(const GlobalStateManager&) = delete;

  void InitStates();

  struct {
    GLViewport viewport;
    GLProgram program;
    GLScissorTest scissor;
    GLScissorRegion scissor_rect;
    GLBlend blend;
    GLBlendFunc blend_func;
    GLClearColor clear_color;
  } states;

  std::unique_ptr<GLShaderWare> shaders;

  std::unique_ptr<QuadIndexBuffer> quad_ibo;
};

}  // namespace gpu

#endif  // GPU_GLES2_GSM_GLES_GSM_H_