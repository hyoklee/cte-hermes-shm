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

#ifndef HSHM_BENCHMARK_DATA_STRUCTURE_TEST_INIT_H_
#define HSHM_BENCHMARK_DATA_STRUCTURE_TEST_INIT_H_

#include <boost/container/scoped_allocator.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#include "hermes_shm/data_structures/all.h"
#include "hermes_shm/util/timer.h"

using hshm::ipc::Allocator;
using hshm::ipc::AllocatorId;
using hshm::ipc::AllocatorType;
using hshm::ipc::MemoryBackend;
using hshm::ipc::MemoryBackendType;
using hshm::ipc::Pointer;

using hshm::ipc::Allocator;
using hshm::ipc::AllocatorId;
using hshm::ipc::AllocatorType;
using hshm::ipc::MemoryBackend;
using hshm::ipc::MemoryBackendType;
using hshm::ipc::MemoryManager;
using hshm::ipc::Pointer;

using Timer = hshm::HighResMonotonicTimer;

extern const std::string shm_url;

#endif  // HSHM_BENCHMARK_DATA_STRUCTURE_TEST_INIT_H_
