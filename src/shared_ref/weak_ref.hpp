#pragma once

#include <cstddef>

#include "internal/control_block.hpp"
#include "shared_ref.hpp"

namespace sm {
    template<typename T>
    class WeakRef {
    public:
        WeakRef() noexcept = default;

        WeakRef(std::nullptr_t) noexcept {}

        WeakRef(SharedRef<T>& shared_pointer) noexcept
            : block(shared_pointer.block), object_pointer(shared_pointer.object_pointer) {
            if (block != nullptr) {
                block->weak_count++;
            }
        }

        ~WeakRef() noexcept {
            destroy_this();
        }

        WeakRef(const WeakRef& other) noexcept
            : block(other.block), object_pointer(other.object_pointer) {
            if (block != nullptr) {
                block->weak_count++;
            }
        }

        WeakRef<T>& operator=(const WeakRef& other) noexcept {
            destroy_this();

            block = other.block;
            object_pointer = other.object_pointer;

            if (block != nullptr) {
                block->weak_count++;
            }

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

        WeakRef& operator=(std::nullptr_t) {
            destroy_this();

            block = nullptr;
            object_pointer = nullptr;

            return *this;
        }

        void reset() noexcept {
            destroy_this();

            block = nullptr;
            object_pointer = nullptr;
        }

        std::size_t use_count() const noexcept {
            if (block == nullptr) {
                return 0;
            }

            return block->ref_count;
        }

        bool expired() const noexcept {
            if (block == nullptr) {
                return true;
            }

            return block->ref_count == 0;
        }

        SharedRef<T> lock() {
            SharedRef<T> strong_ref;

            if (!expired()) {
                strong_ref.block = block;
                strong_ref.object_pointer = object_pointer;

                block->ref_count++;
            }

            return strong_ref;
        }
    private:
        void destroy_this() noexcept {
            if (block == nullptr) {
                return;
            }

            block->weak_count--;
        }

        internal::ControlBlock* block = nullptr;
        T* object_pointer = nullptr;  // This is always null or something
    };
}
