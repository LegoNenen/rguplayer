// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/renderer_worker.h"

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "SDL_hints.h"
#include "content/config/core_config.h"
#include "renderer/context/gles2_context.h"
#include "renderer/states/draw_states.h"
#include "renderer/thread/thread_manager.h"

namespace content {

namespace {

std::vector<SDL_EGLAttrib> g_angle_attrib;
SDL_EGLAttrib* SDLCALL GetANGLEAttribArray() {
  return g_angle_attrib.data();
}

}  // namespace

void RenderRunner::InitRenderer(scoped_refptr<CoreConfigure> config,
                                base::WeakPtr<ui::Widget> host_window) {
  config_ = config;
  host_window_ = host_window;

  InitGLContextInternal();
}

void RenderRunner::DestroyRenderer() {
  QuitGLContextInternal();
}

void RenderRunner::InitANGLERenderer(CoreConfigure::ANGLERenderer renderer) {
  SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");

  if (renderer == content::CoreConfigure::DefaultGLES)
    return;

  g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
  switch (renderer) {
    default:
    case content::CoreConfigure::D3D9:
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE);
      break;
    case content::CoreConfigure::D3D11:
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);
      break;
    case content::CoreConfigure::Vulkan:
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE);
      break;
    case content::CoreConfigure::Metal:
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE);
      break;
    case content::CoreConfigure::Software:
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE);
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
      g_angle_attrib.push_back(
          EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE);
      break;
  }
  g_angle_attrib.push_back(EGL_NONE);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_EGL_PLATFORM, EGL_PLATFORM_ANGLE_ANGLE);
  SDL_EGL_SetEGLAttributeCallbacks(GetANGLEAttribArray, nullptr, nullptr);
}

void RenderRunner::InitGLContextInternal() {
  glcontext_ = SDL_GL_CreateContext(host_window_->AsSDLWindow());
  SDL_GL_MakeCurrent(host_window_->AsSDLWindow(), glcontext_);
  SDL_GL_SetSwapInterval(0);

  renderer::GLES2Context::CreateForCurrentThread();

  if (config_->renderer_debug_output())
    renderer::GLES2Context::EnableDebugOutputForCurrentThread();

  renderer::GSM.InitStates();
  max_texture_size_ = renderer::GSM.GetMaxTextureSize();

  LOG(INFO) << "[Content] GLRenderer: " << renderer::GL.GetString(GL_RENDERER);
  LOG(INFO) << "[Content] GLVendor: " << renderer::GL.GetString(GL_VENDOR);
  LOG(INFO) << "[Content] GLVersion: " << renderer::GL.GetString(GL_VERSION);
  LOG(INFO) << "[Content] GLSL: "
            << renderer::GL.GetString(GL_SHADING_LANGUAGE_VERSION);
  LOG(INFO) << "[Content] MaxTextureSize: " << max_texture_size_ << "x"
            << max_texture_size_;

  renderer::GL.Clear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(host_window_->AsSDLWindow());
}

void RenderRunner::QuitGLContextInternal() {
  renderer::GSM.QuitStates();

  SDL_GL_DeleteContext(glcontext_);
  glcontext_ = nullptr;
}

}  // namespace content
