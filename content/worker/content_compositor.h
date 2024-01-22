// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_CONTENT_COMPOSITOR_H_
#define CONTENT_WORKER_CONTENT_COMPOSITOR_H_

#include "content/worker/binding_worker.h"
#include "content/worker/event_runner.h"
#include "content/worker/renderer_worker.h"

#include "content/worker/content_params.h"

namespace content {

class WorkerTreeCompositor {
 public:
  WorkerTreeCompositor();
  ~WorkerTreeCompositor();

  WorkerTreeCompositor(const WorkerTreeCompositor&) = delete;
  WorkerTreeCompositor& operator=(const WorkerTreeCompositor&) = delete;

  void InitCC(const ContentInitParams& params);
  void ContentMain();

 private:
  scoped_refptr<EventRunner> event_runner_;
  scoped_refptr<RenderRunner> render_runner_;
  scoped_refptr<BindingRunner> binding_runner_;
};

}  // namespace content

#endif  // !CONTENT_WORKER_CONTENT_COMPOSITOR_H_
