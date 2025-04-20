#ifndef HSHM_UTIL_GPU_API_H
#define HSHM_UTIL_GPU_API_H

#include "hermes_shm/constants/macros.h"
#include "hermes_shm/util/logging.h"

namespace hshm {

struct GpuIpcMemHandle {
#ifdef HSHM_ENABLE_CUDA
  cudaIpcMemHandle_t cuda_;
#endif
#ifdef HSHM_ENABLE_ROCM
  hipIpcMemHandle_t rocm_;
#endif
};

class GpuApi {
 public:
  static void SetDevice(int gpu_id) {
#if defined(HSHM_ENABLE_CUDA)
    CUDA_ERROR_CHECK(cudaSetDevice(gpu_id));
#elif defined(HSHM_ENABLE_ROCM)
    HIP_ERROR_CHECK(hipSetDevice(gpu_id));
#endif
  }

  static int GetDeviceCount() {
    int ngpu;
#ifdef HSHM_ENABLE_ROCM
    HIP_ERROR_CHECK(hipGetDeviceCount(&ngpu));
#endif
#ifdef HSHM_ENABLE_CUDA
    CUDA_ERROR_CHECK(cudaGetDeviceCount(&ngpu));
#endif
    return ngpu;
  }

  static void Synchronize() {
#ifdef HSHM_ENABLE_ROCM
    HIP_ERROR_CHECK(hipDeviceSynchronize());
#endif
#ifdef HSHM_ENABLE_CUDA
    CUDA_ERROR_CHECK(cudaDeviceSynchronize());
#endif
  }

  static void GetIpcMemHandle(GpuIpcMemHandle &ipc, void *data) {
#ifdef HSHM_ENABLE_ROCM
    HIP_ERROR_CHECK(hipIpcGetMemHandle(&ipc.rocm_, (void *)data));
#endif
#ifdef HSHM_ENABLE_CUDA
    CUDA_ERROR_CHECK(cudaIpcGetMemHandle(&ipc.cuda_, (void *)data));
#endif
  }

  template <typename T>
  static void OpenIpcMemHandle(GpuIpcMemHandle &ipc, T **data) {
#ifdef HSHM_ENABLE_ROCM
    HIP_ERROR_CHECK(hipIpcOpenMemHandle((void **)data, ipc.rocm_,
                                        hipIpcMemLazyEnablePeerAccess));
#endif
#ifdef HSHM_ENABLE_CUDA
    CUDA_ERROR_CHECK(cudaIpcOpenMemHandle((void **)data, ipc.cuda_,
                                          cudaIpcMemLazyEnablePeerAccess));
#endif
  }

  template <typename T>
  static T *Malloc(size_t size) {
#ifdef HSHM_ENABLE_ROCM
    T *ptr;
    HIP_ERROR_CHECK(hipMalloc(&ptr, size));
    return ptr;
#endif
#ifdef HSHM_ENABLE_CUDA
    T *ptr;
    CUDA_ERROR_CHECK(cudaMalloc(&ptr, size));
    return ptr;
#endif
  }

  template <typename T>
  static void RegisterHostMemory(T *ptr, size_t size) {
#ifdef HSHM_ENABLE_ROCM
    HIP_ERROR_CHECK(
        hipHostRegister((void *)ptr, size, hipHostRegisterPortable));
#endif
#ifdef HSHM_ENABLE_CUDA
    CUDA_ERROR_CHECK(
        cudaHostRegister((void *)ptr, size, cudaHostRegisterPortable));
#endif
  }

  template <typename T>
  static void UnregisterHostMemory(T *ptr) {
#ifdef HSHM_ENABLE_ROCMs
    HIP_ERROR_CHECK(hipHostUnregister((void *)ptr));
#endif
#ifdef HSHM_ENABLE_CUDA
    CUDA_ERROR_CHECK(cudaHostUnregister((void *)ptr));
#endif
  }

  template <typename T>
  static void Memcpy(T *dst, T *src, size_t size) {
#ifdef HSHM_ENABLE_ROCM
    HIP_ERROR_CHECK(hipMemcpy(dst, src, size, hipMemcpyDefault));
#endif
#ifdef HSHM_ENABLE_CUDA
    CUDA_ERROR_CHECK(cudaMemcpy(dst, src, size, cudaMemcpyDefault));
#endif
  }

  template <typename T>
  static bool IsDevicePointer(T *ptr) {
#ifdef HSHM_ENABLE_ROCM
    hipPointerAttribute_t attributes;
    HIP_ERROR_CHECK(hipPointerGetAttributes(&attributes, (void *)ptr));
    return attributes.type == hipMemoryTypeDevice;
#endif
#ifdef HSHM_ENABLE_CUDA
    cudaPointerAttributes attributes;
    CUDA_ERROR_CHECK(cudaPointerGetAttributes(&attributes, (void *)ptr));
    return attributes.type == cudaMemoryTypeDevice;
#endif
    return false;
  }

  template <typename T>
  static void Memset(T *dst, int value, size_t size) {
    if (IsDevicePointer(dst)) {
#ifdef HSHM_ENABLE_ROCM
      HIP_ERROR_CHECK(hipMemset(dst, value, size));
#endif
#ifdef HSHM_ENABLE_CUDA
      CUDA_ERROR_CHECK(cudaMemset(dst, value, size));
#endif
    } else {
      memset(dst, value, size);
    }
  }

  template <typename T>
  static void Free(T *ptr) {
#ifdef HSHM_ENABLE_ROCM
    HIP_ERROR_CHECK(hipFree(ptr));
#endif
#ifdef HSHM_ENABLE_CUDA
    CUDA_ERROR_CHECK(cudaFree(ptr));
#endif
  }
};

}  // namespace hshm

#endif