// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_BINDING_WORKER_H_
#define CONTENT_WORKER_BINDING_WORKER_H_

#include "base/memory/ref_counted.h"
#include "content/public/graphics.h"
#include "content/public/input.h"
#include "content/worker/content_params.h"

#include <thread>

namespace content {

class BindingRunner : public base::RefCounted<BindingRunner> {
 public:
  BindingRunner() = default;

  BindingRunner(const BindingRunner&) = delete;
  BindingRunner& operator=(const BindingRunner&) = delete;

  void InitBindingComponents(ContentInitParams& params);
  void BindingMain(uint32_t event_id);
  void RequestQuit();
  bool CheckQuitFlag();

  int rgss_version() { return config_->content_version(); }
  scoped_refptr<CoreConfigure> config() const { return config_; }
  scoped_refptr<Graphics> graphics() const { return graphics_; }
  scoped_refptr<Input> input() const { return input_; }
  uint32_t user_event_id() { return user_event_id_; }

 private:
  static void BindingFuncMain(base::WeakPtr<BindingRunner> self);

  scoped_refptr<CoreConfigure> config_;
  std::unique_ptr<std::thread> runner_thread_;

  scoped_refptr<RenderRunner> renderer_;
  scoped_refptr<Graphics> graphics_;
  scoped_refptr<Input> input_;

  base::Vec2i initial_resolution_;
  base::WeakPtr<ui::Widget> window_;
  base::AtomicFlag quit_atomic_;
  uint32_t user_event_id_;

  std::unique_ptr<BindingEngine> binding_engine_;

  base::WeakPtrFactory<BindingRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  //! CONTENT_WORKER_BINDING_WORKER_H_
