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

#ifndef HSHM_TEST_UNIT_DATA_STRUCTURES_CONTAINERS_LIST_H_
#define HSHM_TEST_UNIT_DATA_STRUCTURES_CONTAINERS_LIST_H_

#include "basic_test.h"
#include "hermes_shm/data_structures/ipc/string.h"
#include "test_init.h"

template <typename T, typename Container,
          typename AllocT = HSHM_DEFAULT_ALLOC_T>
class ListTestSuite {
 public:
  Container &obj_;
  AllocT *alloc_;

  /// Constructor
  ListTestSuite(Container &obj, AllocT *alloc) : obj_(obj), alloc_(alloc) {}

  /// Emplace elements
  void EmplaceTest(size_t count = 30) {
    for (size_t i = 0; i < count; ++i) {
      CREATE_SET_VAR_TO_INT_OR_STRING(T, var, i);
      obj_.emplace_back(var);
    }
    REQUIRE(obj_.size() == count);
  }

  /// Forward iterator
  void ForwardIteratorTest(size_t count = 30) {
    size_t fcur = 0;
    for (auto &num : obj_) {
      CREATE_SET_VAR_TO_INT_OR_STRING(T, fcur_conv, fcur);
      REQUIRE(num == fcur_conv);
      ++fcur;
    }
  }

  /// Constant Forward iterator
  void ConstForwardIteratorTest(size_t count = 30) {
    const Container &obj = obj_;
    size_t fcur = 0;
    for (auto iter = obj.cbegin(); iter != obj.cend(); ++iter) {
      T &num = *iter;
      CREATE_SET_VAR_TO_INT_OR_STRING(T, fcur_conv, fcur);
      REQUIRE(num == fcur_conv);
      ++fcur;
    }
  }

  /// Copy constructor
  void CopyConstructorTest() {
    size_t count = obj_.size();
    Container cpy(obj_);
    VerifyCopy(obj_, cpy, count);
  }

  /// Copy assignment
  void CopyAssignmentTest() {
    size_t count = obj_.size();
    Container cpy;
    cpy = obj_;
    VerifyCopy(obj_, cpy, count);
  }

  /// Move constructor
  void MoveConstructorTest() {
    size_t count = obj_.size();
    Container cpy(std::move(obj_));
    VerifyMove(obj_, cpy, count);
    obj_ = std::move(cpy);
    VerifyMove(cpy, obj_, count);
  }

  /// Move assignment
  void MoveAssignmentTest() {
    size_t count = obj_.size();
    Container cpy;
    cpy = std::move(obj_);
    VerifyMove(obj_, cpy, count);
    obj_ = std::move(cpy);
    VerifyMove(cpy, obj_, count);
  }

  /// Emplace and erase front
  void EmplaceFrontTest() {
    CREATE_SET_VAR_TO_INT_OR_STRING(T, i0, 100);
    size_t old_size = obj_.size();
    obj_.emplace_front(i0);
    REQUIRE(obj_.front() == i0);
    REQUIRE(obj_.size() == old_size + 1);
    obj_.erase(obj_.begin(), obj_.begin() + 1);
  }

  /// Copy an object into the container
  void ModifyEntryCopyIntoTest() {
    // Modify the fourth list entry
    {
      CREATE_SET_VAR_TO_INT_OR_STRING(T, i4, 25);
      auto iter = obj_.begin() + 4;
      (*iter) = i4;
    }

    // Verify the modification took place
    {
      CREATE_SET_VAR_TO_INT_OR_STRING(T, i4, 25);
      auto iter = obj_.begin() + 4;
      REQUIRE((*iter) == i4);
    }
  }

  /// Move an object into the container
  void ModifyEntryMoveIntoTest() {
    // Modify the fourth list entry
    {
      CREATE_SET_VAR_TO_INT_OR_STRING(T, i4, 25);
      auto iter = obj_.begin() + 4;
      (*iter) = std::move(i4);
    }

    // Verify the modification took place
    {
      CREATE_SET_VAR_TO_INT_OR_STRING(T, i4, 25);
      auto iter = obj_.begin() + 4;
      REQUIRE((*iter) == i4);
    }
  }

  /// Verify erase
  void EraseTest() {
    obj_.clear();
    REQUIRE(obj_.size() == 0);
  }

 private:
  /// Verify copy construct/assign worked
  void VerifyCopy(Container &obj, Container &cpy, size_t count) {
    REQUIRE(obj_.size() == count);
    REQUIRE(cpy.size() == count);

    // Verify obj
    {
      size_t fcur = 0;
      for (auto &num : obj_) {
        CREATE_SET_VAR_TO_INT_OR_STRING(T, fcur_conv, fcur);
        REQUIRE(num == fcur_conv);
        ++fcur;
      }
    }

    // Verify copy
    {
      size_t fcur = 0;
      for (auto &num : cpy) {
        CREATE_SET_VAR_TO_INT_OR_STRING(T, fcur_conv, fcur);
        REQUIRE(num == fcur_conv);
        ++fcur;
      }
    }
  }

  /// Verify move worked
  void VerifyMove(Container &orig_obj, Container &new_obj, size_t count) {
    // Verify move into cpy worked
    {
      size_t fcur = 0;
      REQUIRE(orig_obj.IsNull());
      REQUIRE(new_obj.size() == count);
      for (auto &num : new_obj) {
        CREATE_SET_VAR_TO_INT_OR_STRING(T, fcur_conv, fcur);
        REQUIRE(num == fcur_conv);
        ++fcur;
      }
    }
  }
};

#endif  // HSHM_TEST_UNIT_DATA_STRUCTURES_CONTAINERS_LIST_H_
