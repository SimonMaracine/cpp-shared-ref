#pragma once

#include <cstddef>

#include "internal/control_block.hpp"
#include "shared_ref.hpp"

namespace sm {
    template<typename T>
    class WeakRef {
    public:
        WeakRef() noexcept = default;

        WeakRef(SharedRef<T>& shared_pointer) noexcept
            : block(shared_pointer.block), object_pointer(shared_pointer.object_pointer) {
            block->weak_count++;
        }

        ~WeakRef() noexcept {
            destroy_this();
        }

        WeakRef(const WeakRef& other) noexcept
            : block(other.block), object_pointer(other.object_pointer) {
            block->weak_count++;
        }

        WeakRef<T>& operator=(const WeakRef& other) noexcept {
            destroy_this();

            block = other.block;
            object_pointer = other.object_pointer;

            block->weak_count++;

            return *this;
        }

        WeakRef(WeakRef&& other) noexcept
            : block(other.block), object_pointer(other.object_pointer) {
            other.block = nullptr;
            other.object_pointer = nullptr;
        }

        WeakRef<T>& operator=(WeakRef&& other) noexcept {
            destroy_this();

            block = other.block;
            object_pointer = other.object_pointer;

            other.block = nullptr;
            other.object_pointer = nullptr;

            return *this;
        }

        void reset() noexcept {
            destroy_this();

            block = nullptr;
            object_pointer = nullptr;
        }

        std::size_t use_count() const noexcept {
            return block->ref_count;
        }

        bool expired() const noexcept {
            return block->ref_count == 0;
        }

        SharedRef<T> lock() {
            SharedRef<T> strong_pointer;

            if (!expired()) {
                strong_pointer.block = block;
                strong_pointer.object_pointer = object_pointer;

                block->ref_count++;
            }

            return strong_pointer;
        }
    private:
        void destroy_this() noexcept {
            if (block == nullptr) {
                return;
            }

            block->weak_count--;
        }

        internal::ControlBlock<T>* block = nullptr;
        T* object_pointer = nullptr;  // This is always null or something
    };
}
