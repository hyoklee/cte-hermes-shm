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


#include <hermes_shm/memory/allocator/fixed_page_allocator.h>
#include <hermes_shm/memory/allocator/mp_page.h>

namespace hermes::ipc {

void FixedPageAllocator::shm_init(allocator_id_t id,
                                  size_t custom_header_size,
                                  char *buffer,
                                  size_t buffer_size) {
  buffer_ = buffer;
  buffer_size_ = buffer_size;
  header_ = reinterpret_cast<FixedPageAllocatorHeader*>(buffer_);
  custom_header_ = reinterpret_cast<char*>(header_ + 1);
  size_t region_off = (custom_header_ - buffer_) + custom_header_size;
  size_t region_size = buffer_size_ - region_off;
  alloc_.shm_init(id, 0, buffer + region_off, region_size);
  header_->Configure(id, custom_header_size, &alloc_);
  free_lists_->shm_deserialize(header_->free_lists_.internal_ref(&alloc_));
}

void FixedPageAllocator::shm_deserialize(char *buffer,
                                         size_t buffer_size) {
  buffer_ = buffer;
  buffer_size_ = buffer_size;
  header_ = reinterpret_cast<FixedPageAllocatorHeader*>(buffer_);
  custom_header_ = reinterpret_cast<char*>(header_ + 1);
  size_t region_off = (custom_header_ - buffer_) + header_->custom_header_size_;
  size_t region_size = buffer_size_ - region_off;
  alloc_.shm_deserialize(buffer + region_off, region_size);
  free_lists_->shm_deserialize(header_->free_lists_.internal_ref(&alloc_));
}

size_t FixedPageAllocator::GetCurrentlyAllocatedSize() {
  return header_->total_alloc_;
}

OffsetPointer FixedPageAllocator::AllocateOffset(size_t size) {
  size += sizeof(MpPage);
  MpPage *page = nullptr;

  // Check if page of this size is already cached
  for (hipc::ShmRef<list<OffsetPointer>> free_list : *free_lists_) {
    if (free_list->size()) {
      auto iter = free_list->begin();
      hipc::ShmRef<OffsetPointer> page_ref = *iter;
      page = Convert<MpPage>(*page_ref);
      if (page->page_size_ != size) {
        continue;
      }
      free_list->erase(iter);
    }
  }

  // Allocate from stack if no page found
  if (page == nullptr){
    page = alloc_.AllocatePtr<MpPage>(size);
  }
  if (page == nullptr) {
    throw OUT_OF_MEMORY;
  }

  // Mark as allocated
  header_->total_alloc_.fetch_add(size);
  auto p = Convert<MpPage, OffsetPointer>(page);
  return p;
}

OffsetPointer FixedPageAllocator::AlignedAllocateOffset(size_t size,
                                                        size_t alignment) {
  throw ALIGNED_ALLOC_NOT_SUPPORTED.format();
}

OffsetPointer FixedPageAllocator::ReallocateOffsetNoNullCheck(OffsetPointer p,
                                                          size_t new_size) {
  throw ALIGNED_ALLOC_NOT_SUPPORTED.format();
}

void FixedPageAllocator::FreeOffsetNoNullCheck(OffsetPointer p) {
  // Mark as free
  auto hdr_offset = p - sizeof(MpPage);
  auto hdr = Convert<MpPage>(hdr_offset);
  if (!hdr->IsAllocated()) {
    throw DOUBLE_FREE.format();
  }
  hdr->UnsetAllocated();
  header_->total_alloc_.fetch_sub(hdr->page_size_);

  // Append to a free list
  for (hipc::ShmRef<list<OffsetPointer>> free_list : *free_lists_) {
    if (free_list->size()) {
      auto iter = free_list->begin();
      hipc::ShmRef<OffsetPointer> page_ref = *iter;
      auto page = Convert<MpPage>(*page_ref);
      if (page->page_size_ != hdr->page_size_) {
        continue;
      }
    }
    free_list->emplace_back(hdr_offset);
    return;
  }

  // Extend the set of cached pages
  free_lists_->emplace_back();
  hipc::ShmRef<list<OffsetPointer>> free_list =
    (*free_lists_)[free_lists_->size() - 1];
  free_list->emplace_back(hdr_offset);
}

}  // namespace hermes::ipc
