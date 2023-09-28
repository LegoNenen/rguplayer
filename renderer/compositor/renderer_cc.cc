// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/compositor/renderer_cc.h"

namespace renderer {

CCLayer::CCLayer(SDL_Window* window, const SDL_GLContext& gl_ctx)
    : gl_sdl_ctx_(gl_ctx), window_(window) {
  context_ = base::MakeRefCounted<gpu::GLES2CommandContext>();
  context_->InitContext();

  quad_indices_buffer_ = base::MakeRefCounted<QuadIndicesBuffer>(context_);

  states.viewport_ = std::make_unique<GLViewport>(context_);
  states.scissor_region_ = std::make_unique<GLScissorRegion>(context_);
  states.scissor_test_ = std::make_unique<GLScissorTest>(context_);
  states.blend_mode_ = std::make_unique<GLBlendMode>(context_);

  shaders.simple_shader = std::make_unique<gpu::SimpleShader>(context_);
}

}  // namespace renderer
