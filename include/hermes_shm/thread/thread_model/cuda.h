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

#ifndef HERMES_SHM_INCLUDE_HERMES_SHM_THREAD_CUDA_H__
#define HERMES_SHM_INCLUDE_HERMES_SHM_THREAD_CUDA_H__

#include "thread_model.h"
#include <thallium.hpp>
#include <errno.h>
#include "hermes_shm/util/errors.h"
#include <omp.h>
#include "hermes_shm/introspect/system_info.h"

namespace hshm::thread_model {

class Cuda : public ThreadModel {
 public:
  /** Default constructor */
  HSHM_CROSS_FUN
  Cuda() = default;

  /** Virtual destructor */
  HSHM_CROSS_FUN
  virtual ~Cuda() = default;

  /** Yield the current thread for a period of time */
  HSHM_CROSS_FUN
  void SleepForUs(size_t us) override {
  }

  /** Yield thread time slice */
  HSHM_CROSS_FUN
  void Yield() override {
  }

  /** Get the TID of the current thread */
  ThreadId GetTid() override {
    return ThreadId::GetNull();
  }
};

}  // namespace hshm::thread_model

#endif  // HERMES_SHM_INCLUDE_HERMES_SHM_THREAD_CUDA_H__
