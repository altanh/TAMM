#pragma once

#include "tamm/gpu_streams.hpp"

#include <cstddef>
#include <new>
#include <unordered_map>
#include <vector>

namespace tamm {

class GPUPooledStorageManager {
protected:
  // used memory
  size_t used_memory_ = 0;
  // percentage of reserved memory
  int reserve_;
  // memory pool
  std::unordered_map<size_t, std::vector<void*>> memory_pool_;

private:
  GPUPooledStorageManager() { reserve_ = 5; }
  ~GPUPooledStorageManager() { ReleaseAll(); }

public:
  void* allocate(size_t size) {
    auto&& reuse_it = memory_pool_.find(size);
    if(reuse_it == memory_pool_.end() || reuse_it->second.size() == 0) {
      size_t free{}, total{};

#if defined(USE_CUDA)
      cudaMemGetInfo(&free, &total);
#elif defined(USE_HIP)
      hipMemGetInfo(&free, &total);
#elif defined(USE_DPCPP)
      syclMemGetInfo(&free, &total);
#endif

      if(size > free - total * reserve_ / 100) ReleaseAll();

      void* ret = nullptr;

#if defined(USE_CUDA)
      cudaMalloc(&ret, size);
#elif defined(USE_HIP)
      hipMalloc(&ret, size);
#elif defined(USE_DPCPP)
      auto&        streamPool = tamm::GPUStreamPool::getInstance();
      gpuStream_t& stream     = streamPool.getStream();
      ret                     = sycl::malloc_device(size, stream);
#endif

      used_memory_ += size;
      return ret;
    }
    else {
      auto&& reuse_pool = reuse_it->second;
      auto   ret        = reuse_pool.back();
      reuse_pool.pop_back();
      return ret;
    }
  }
  void deallocate(void* ptr, size_t size) {
    auto&& reuse_pool = memory_pool_[size];
    reuse_pool.push_back(ptr);
  }

  void ReleaseAll() {
    for(auto&& i: memory_pool_) {
      for(auto&& j: i.second) {
#if defined(USE_CUDA)
        cudaFree(j);
#elif defined(USE_HIP)
        hipFree(j);
#elif defined(USE_DPCPP)
        auto&        streamPool = tamm::GPUStreamPool::getInstance();
        gpuStream_t& stream     = streamPool.getStream();
        sycl::free(j, stream);
#endif
        used_memory_ -= i.first;
      }
    }
    memory_pool_.clear();
  }

  /// Returns the instance of device manager singleton.
  inline static GPUPooledStorageManager& getInstance() {
    static GPUPooledStorageManager d_m{};
    return d_m;
  }

  GPUPooledStorageManager(const GPUPooledStorageManager&)            = delete;
  GPUPooledStorageManager& operator=(const GPUPooledStorageManager&) = delete;
  GPUPooledStorageManager(GPUPooledStorageManager&&)                 = delete;
  GPUPooledStorageManager& operator=(GPUPooledStorageManager&&)      = delete;

}; // class GPUPooledStorageManager

} // namespace tamm
