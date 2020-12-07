//
// Created by jiashuai on 17-9-16.
//

#include <thundergbm/syncmem.h>

namespace thunder {
    SyncMem::SyncMem() : SyncMem(0) {}

    SyncMem::SyncMem(size_t size) : device_ptr(nullptr), host_ptr(nullptr), size_(size), head_(UNINITIALIZED),
                                    own_device_data(false), own_host_data(false) {
#ifdef USE_CUDA
        CUDA_CHECK(cudaGetDevice(&device_id));
#endif
    }

    SyncMem::~SyncMem() {
        this->head_ = UNINITIALIZED;
        if (host_ptr && own_host_data) {
            free_host(host_ptr);
            host_ptr = nullptr;
        }
#ifdef USE_CUDA
        DO_ON_DEVICE(device_id, {
            if (device_ptr && own_device_data) {
                CUDA_CHECK(cudaFree(device_ptr));
                device_ptr = nullptr;
            }
        });
#endif
    }

    void *SyncMem::host_data() {
        to_host();
        return host_ptr;
    }

    void *SyncMem::device_data() {
#ifdef USE_CUDA
        to_device();
#else
        NO_GPU;
#endif
        return device_ptr;
    }

    size_t SyncMem::size() const {
        return size_;
    }

    SyncMem::HEAD SyncMem::head() const {
        return head_;
    }

    void SyncMem::to_host() {
        switch (head_) {
            case UNINITIALIZED:
                malloc_host(&host_ptr, size_);
                memset(host_ptr, 0, size_);
                head_ = HOST;
                own_host_data = true;
                break;
            case DEVICE:
#ifdef USE_CUDA
                DO_ON_DEVICE(device_id, {
                    if (nullptr == host_ptr) {
                        CUDA_CHECK(cudaHostAlloc(&host_ptr, size_, cudaHostAllocPortable));
                        CUDA_CHECK(cudaMemset(host_ptr, 0, size_));
                        own_host_data = true;
                    }
                    CUDA_CHECK(cudaMemcpy(host_ptr, device_ptr, size_, cudaMemcpyDeviceToHost));
                    head_ = HOST;
                });
#else
                NO_GPU;
#endif
                break;
            case HOST:;
        }
    }

    void SyncMem::to_device() {
#ifdef USE_CUDA
        DO_ON_DEVICE(device_id, {
            switch (head_) {
                case UNINITIALIZED:
                    CUDA_CHECK(cudaMalloc(&device_ptr, size_));
                    CUDA_CHECK(cudaMemset(device_ptr, 0, size_));
                    head_ = DEVICE;
                    own_device_data = true;
                    break;
                case HOST:
                    if (nullptr == device_ptr) {
                        CUDA_CHECK(cudaMalloc(&device_ptr, size_));
                        CUDA_CHECK(cudaMemset(device_ptr, 0, size_));
                        own_device_data = true;
                    }
                    CUDA_CHECK(cudaMemcpy(device_ptr, host_ptr, size_, cudaMemcpyHostToDevice));
                    head_ = DEVICE;
                    break;
                case DEVICE:;
            }
        });
#else
        NO_GPU;
#endif
    }

    void SyncMem::set_host_data(void *data) {
        CHECK_NOTNULL(data);
        if (own_host_data) {
            free_host(host_ptr);
        }
        host_ptr = data;
        own_host_data = false;
        head_ = HEAD::HOST;
    }

    void SyncMem::set_device_data(void *data) {
#ifdef USE_CUDA
        DO_ON_DEVICE(device_id, {
            CHECK_NOTNULL(data);
            if (own_device_data) {
                CUDA_CHECK(cudaFree(device_data()));
            }
            device_ptr = data;
            own_device_data = false;
            head_ = HEAD::DEVICE;
        });
#else
        NO_GPU;
#endif
    }
}
