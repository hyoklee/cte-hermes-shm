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

#ifndef HERMES_ADAPTER_API_H
#define HERMES_ADAPTER_API_H

#include <fcntl.h>
#undef DEPRECATED

#include <dlfcn.h>
#include <link.h>
// #include <libelf.h>
#include <gelf.h>
#include <unistd.h>

#include "singleton.h"

#define HERMES_DECL(F) F

#define REQUIRE_API(api_name)                                            \
  if (!(api_name)) {                                                     \
    HELOG(kFatal, "HERMES Adapter failed to map symbol: {}", #api_name); \
  }

namespace hshm {

struct RealApi {
  const char *symbol_name_;
  const char *intercept_var_;
  const char *real_lib_path_;
  const char *intercepted_lib_path_;
  void *real_lib_;
  void *interceptor_lib_;
  bool is_loaded_ = false;

  /** Locates the interceptor library and real library */
  static int callback(struct dl_phdr_info *info, unsigned long size,
                      void *data) {
    auto iter = (RealApi *)data;
    auto lib = dlopen(info->dlpi_name, RTLD_GLOBAL | RTLD_NOW);
    // auto lib = dlopen(info->dlpi_name, RTLD_NOLOAD | RTLD_NOW);
    if (lib) {
      auto exists = dlsym(lib, iter->symbol_name_);
      void *is_intercepted = (void *)dlsym(lib, iter->intercept_var_);
      if (exists) {
        if (is_intercepted) {
          iter->intercepted_lib_path_ = info->dlpi_name;
        } else if (!iter->real_lib_path_) {
          iter->real_lib_path_ = info->dlpi_name;
        }
      }
    }
    return 0;
  }

  /**
   * @brief Construct a new RealApi object
   *
   * @param symbol_name A function name or variable to look for in the shared
   * object
   * @param intercept_var The name of the variable indicating this is the
   * interceptor object
   */
  RealApi(const char *symbol_name, const char *intercept_var) {
    symbol_name_ = symbol_name;
    intercept_var_ = intercept_var;
    dl_iterate_phdr(callback, (void *)this);
    if (real_lib_path_) {
      real_lib_ = dlopen(real_lib_path_, RTLD_GLOBAL | RTLD_NOW);
    } else {
      real_lib_ = RTLD_DEFAULT;
    }
    if (intercepted_lib_path_) {
      interceptor_lib_ = dlopen(intercepted_lib_path_, RTLD_GLOBAL | RTLD_NOW);
    } else {
      interceptor_lib_ = nullptr;
    }
  }
};

template <typename PosixT>
struct PreloadProgress {
  PosixT *posix_;
  bool is_loaded_;
  int lib_fd_ = -1;
  Elf *lib_elf_ = nullptr;

  /** Load the interceptor lib */
  bool LoadElf(const char *filename) {
    // Check if filename is null
    if (!filename) {
      return false;
    }

    // Open the ELF file
    lib_fd_ = posix_->open(filename, O_RDONLY);
    if (lib_fd_ < 0) {
      return false;
    }

    // Initialize libelf
    if (elf_version(EV_CURRENT) == EV_NONE) {
      return false;
    }

    // Open ELF descriptor
    lib_elf_ = elf_begin(lib_fd_, ELF_C_READ, NULL);
    if (!lib_elf_) {
      return false;
    }

    // Get the ELF header
    GElf_Ehdr ehdr;
    if (!gelf_getehdr(lib_elf_, &ehdr)) {
      return false;
    }

    return true;
  }

  /** Check if all needed libraries have been loaded */
  bool AllDepsLoaded() {
    // Scan the dynamic table
    Elf_Scn *scn = NULL;
    while ((scn = elf_nextscn(lib_elf_, scn)) != NULL) {
      GElf_Shdr shdr = {};
      if (gelf_getshdr(scn, &shdr) != &shdr) {
        return false;
      }

      if (shdr.sh_type == SHT_DYNAMIC) {
        Elf_Data *data = NULL;
        data = elf_getdata(scn, data);
        if (data == NULL) {
          return false;
        }

        size_t sh_entsize = gelf_fsize(lib_elf_, ELF_T_DYN, 1, EV_CURRENT);

        for (size_t i = 0; i < shdr.sh_size / sh_entsize; i++) {
          GElf_Dyn dyn = {};
          if (gelf_getdyn(data, i, &dyn) != &dyn) {
            return false;
          }
          const char *lib_name =
              elf_strptr(lib_elf_, shdr.sh_link, dyn.d_un.d_val);
          if (lib_name) {
            if (dyn.d_tag == DT_NEEDED) {
              void *lib = nullptr;
              // Try direct path first
              lib = dlopen(lib_name, RTLD_NOLOAD | RTLD_NOW);
              if (!lib) {
                // Try with default path resolution
                lib = dlopen(lib_name, RTLD_NOLOAD | RTLD_NOW | RTLD_GLOBAL);
              }
              if (!lib) {
                return false;
              }
            }
          }
        }
      }
    }

    // Clean up
    return true;
  }

  /** Close the elf file */
  void CloseElf() {
    if (lib_elf_) {
      elf_end(lib_elf_);
    }
    if (lib_fd_ > 0) {
      posix_->close(lib_fd_);
    }
  }

  PreloadProgress(RealApi &api) : is_loaded_(false) {
    // char exe_path[1024];
    // ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    // if (len != -1) {
    //   exe_path[len] = '\0';
    // } else {
    //   exe_path[0] = '\0';
    // }

    posix_ = hshm::Singleton<PosixT>::GetInstance();
    if (!LoadElf(api.intercepted_lib_path_)) {
      // if (!LoadElf(exe_path)) {
      return;
    }
    is_loaded_ = AllDepsLoaded();
    CloseElf();
  }
};

}  // namespace hshm

#undef DEPRECATED

#endif  // HERMES_ADAPTER_API_H
