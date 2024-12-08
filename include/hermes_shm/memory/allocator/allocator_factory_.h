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

#ifndef HERMES_MEMORY_ALLOCATOR_ALLOCATOR_FACTORY__H_
#define HERMES_MEMORY_ALLOCATOR_ALLOCATOR_FACTORY__H_

#include "allocator.h"

namespace hshm::ipc {

class _MallocAllocator;
typedef BaseAllocator<_MallocAllocator> MallocAllocator;

class _FixedPageAllocator;
typedef BaseAllocator<_FixedPageAllocator> FixedPageAllocator;

class _ScalablePageAllocator;
typedef BaseAllocator<_ScalablePageAllocator> ScalablePageAllocator;

class _StackAllocator;
typedef BaseAllocator<_StackAllocator> StackAllocator;

}  // namespace hshm::ipc

#endif  // HERMES_MEMORY_ALLOCATOR_ALLOCATOR_FACTORY__H_