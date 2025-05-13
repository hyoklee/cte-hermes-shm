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

#ifndef HSHM_SHM_SHM_DATA_STRUCTURES_CONTAINERS_ALGORITHM_H
#define HSHM_SHM_SHM_DATA_STRUCTURES_CONTAINERS_ALGORITHM_H

#include <hermes_shm/constants/macros.h>
#include <hermes_shm/types/numbers.h>

namespace hshm {

/** Swap two values */
template <typename T>
HSHM_CROSS_FUN void swap(T &a, T &b) {
  T tmp = a;
  a = b;
  b = tmp;
}

/** Default sorting algorithm */
#define HSHM_DEFAULT_SORT_CMP hshm ::less_than<IterT>

/** Greater than comparison */
template <typename IterT>
HSHM_CROSS_FUN bool greater_than(const IterT &a, const IterT &b) {
  return *a > *b;
}

/** Less than comparison */
template <typename IterT>
HSHM_CROSS_FUN bool less_than(const IterT &a, const IterT &b) {
  return *a < *b;
}

/** Comparitor type */
template <typename IterT>
using Comparitor = bool (*)(const IterT &a, const IterT &b);

/** Check if a set of values is sorted */
template <typename IterT, typename CmpT = Comparitor<IterT>>
HSHM_CROSS_FUN bool is_sorted(IterT start, const IterT &end,
                              CmpT &&cmp = HSHM_DEFAULT_SORT_CMP) {
  if (start == end) {
    return true;
  }
  auto prev = start;
  ++start;
  for (; start != end; ++start) {
    if (!cmp(prev, start)) {
      return false;
    }
    ++prev;
  }
  return true;
}

/** General sort forward declaration */
template <typename IterT, typename CmpT = Comparitor<IterT>,
          int INSERT_SORT_CUTOFF = 32>
HSHM_CROSS_FUN void sort(IterT start, const IterT &end,
                         CmpT &&cmp = HSHM_DEFAULT_SORT_CMP);

/** Insertion sort forward declaration */
template <typename IterT, typename CmpT = Comparitor<IterT>>
HSHM_CROSS_FUN void insertion_sort(IterT start, const IterT &end,
                                   CmpT &&cmp = HSHM_DEFAULT_SORT_CMP);

/** heap_sort forward declaration */
template <typename IterT, typename CmpT = Comparitor<IterT>>
HSHM_CROSS_FUN void heap_sort(IterT start, const IterT &end,
                              CmpT &&cmp = HSHM_DEFAULT_SORT_CMP);

/** Quicksort forward declaration */
template <typename IterT, typename CmpT = Comparitor<IterT>>
HSHM_CROSS_FUN void quick_sort(IterT start, const IterT &end,
                               CmpT &&cmp = HSHM_DEFAULT_SORT_CMP);

/** Sort using either insertion sort of quick_sort */
template <typename IterT, typename CmpT, int INSERT_SORT_CUTOFF>
HSHM_CROSS_FUN void sort(IterT start, const IterT &end, CmpT &&cmp) {
  if (end - start < INSERT_SORT_CUTOFF) {
    insertion_sort(start, end, cmp);
  } else {
    quick_sort(start, end, cmp);
  }
}

/** Sort a set of values using insertion sort */
template <typename IterT, typename CmpT>
HSHM_CROSS_FUN void insertion_sort(IterT start, const IterT &end, CmpT &&cmp) {
  if (start == end) {
    return;
  }
  for (auto i = start; i != end; ++i) {
    auto j = i;
    while (j != start && cmp(j, (j - 1))) {
      swap(*j, *(j - 1));
      --j;
    }
  }
}

/** Build a hepa from a vector */
template <typename IterT, typename CmpT>
HSHM_CROSS_FUN void heapify(IterT start, size_t n, size_t i, CmpT &&cmp) {
  size_t largest = i;
  size_t left = 2 * i + 1;
  size_t right = 2 * i + 2;

  if (left < n && cmp(start + largest, start + left)) largest = left;

  if (right < n && cmp(start + largest, start + right)) largest = right;

  if (largest != i) {
    swap(*(start + i), *(start + largest));
    heapify(start, n, largest, cmp);
  }
}

/** Sort a set of values with heap sort */
template <typename IterT, typename CmpT>
HSHM_CROSS_FUN void heap_sort(IterT start, const IterT &end, CmpT &&cmp) {
  size_t n = end - start;

  // Build heap
  for (int i = n / 2 - 1; i >= 0; i--) heapify(start, n, i, cmp);

  // Extract elements from heap one by one
  for (int i = n - 1; i > 0; i--) {
    swap(*start, *(start + i));
    heapify(start, i, 0, cmp);
  }
}

/** Sort a set of values using quick sort */
template <typename IterT, typename CmpT>
HSHM_CROSS_FUN void quick_sort(IterT start, const IterT &end, CmpT &&cmp) {
  if (start >= end - 1) {
    return;
  }
  auto pivot = start + (end - start) / 2;
  auto pivot_val = *pivot;
  swap(*pivot, *(end - 1));
  auto store = start;
  for (auto i = start; i < end - 1; ++i) {
    if (cmp(i, end - 1)) {
      swap(*store, *i);
      ++store;
    }
  }
  swap(*store, *(end - 1));
  sort(start, store, cmp);
  sort(store + 1, end, cmp);
}

}  // namespace hshm

#endif  // HSHM_SHM_SHM_DATA_STRUCTURES_CONTAINERS_ALGORITHM_H
