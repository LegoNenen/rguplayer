// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_WORKER_THREAD_WORKER_H_
#define BASE_WORKER_THREAD_WORKER_H_

#include <thread>

#include "base/bind/callback.h"
#include "base/memory/weak_ptr.h"
#include "base/worker/run_loop.h"

namespace base {

class ThreadWorker {
 public:
  ThreadWorker(bool sync_worker = false);
  virtual ~ThreadWorker();

  ThreadWorker(const ThreadWorker&) = delete;
  ThreadWorker& operator=(const ThreadWorker&) = delete;

  void Start(RunLoop::MessagePumpType message_type);
  void Stop();

  void WaitUntilStart();
  scoped_refptr<base::SequencedTaskRunner> task_runner();

  bool IsSyncMode() { return sync_; }

 private:
  static void ThreadFunc(base::AtomicFlag& token,
                         RunLoop::MessagePumpType message_type,
                         base::AtomicFlag& start_flag,
                         scoped_refptr<base::SequencedTaskRunner>& runner);
  std::unique_ptr<std::thread> thread_;

  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  base::AtomicFlag start_flag_;
  base::AtomicFlag stop_flag_;
  bool sync_;

  base::WeakPtrFactory<ThreadWorker> weak_ptr_factory_{this};
};

}  // namespace base

#endif  // BASE_WORKER_THREAD_WORKER_H_