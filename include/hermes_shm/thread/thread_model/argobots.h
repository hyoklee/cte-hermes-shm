/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Distributed under BSD 3-Clause license.                                   *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Illinois Institute of Technology.                        *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Hermes. The full Hermes copyright notice, including  *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the top directory. If you do not  *
 * have access to the file, you may request a copy from help@hdfgroup.org.   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef HERMES_SHM_INCLUDE_HERMES_SHM_THREAD_THALLIUM_H_
#define HERMES_SHM_INCLUDE_HERMES_SHM_THREAD_THALLIUM_H_

#include "thread_model.h"
#include <thallium.hpp>
#include <errno.h>
#include "hermes_shm/util/errors.h"
#include <omp.h>
#include "hermes_shm/introspect/system_info.h"

namespace hshm::thread_model {

class Argobots : public ThreadModel {
 public:
  /** Default constructor */
  HSHM_CROSS_FUN
  Argobots() = default;

  /** Virtual destructor */
  HSHM_CROSS_FUN
  virtual ~Argobots() = default;

  /** Yield the current thread for a period of time */
  HSHM_CROSS_FUN
  void SleepForUs(size_t us) override {
    /**
     * TODO(llogan): make this API flexible enough to support argobots fully
     * tl::thread::self().sleep(*HERMES->rpc_.server_engine_,
                               HERMES->server_config_.borg_.blob_reorg_period_);
     */
#ifndef __CUDA_ARCH__
    usleep(us);
#endif
  }

  /** Yield thread time slice */
  HSHM_CROSS_FUN
  void Yield() override {
#ifndef __CUDA_ARCH__
    ABT_thread_yield();
#endif
  }

  /** Get the TID of the current thread */
  ThreadId GetTid() override {
    ABT_thread thread;
    ABT_thread_id tid;
    ABT_thread_self(&thread);
    ABT_thread_get_id(thread, &tid);
    return ThreadId{tid};
  }
};

}  // namespace hshm::thread_model

#endif  // HERMES_SHM_INCLUDE_HERMES_SHM_THREAD_THALLIUM_H_
