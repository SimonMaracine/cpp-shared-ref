#pragma once

#include <cstddef>

#include "internal/control_block.hpp"
#include "shared_ref.hpp"

namespace sm {
    template<typename T>
    class weak_ref {
    public:
        weak_ref() noexcept = default;
        weak_ref(std::nullptr_t) noexcept {}

        weak_ref(shared_ref<T>& shared_pointer) noexcept
            : block(shared_pointer.block), object_pointer(shared_pointer.object_pointer) {
            if (block != nullptr) {
                block->weak_count++;
            }
        }

        weak_ref& operator=(std::nullptr_t) noexcept {
            destroy_this();

            block = nullptr;
            object_pointer = nullptr;

            return *this;
        }

        ~weak_ref() noexcept {
            destroy_this();
        }

        weak_ref(const weak_ref& other) noexcept
            : block(other.block), object_pointer(other.object_pointer) {
            if (block != nullptr) {
                block->weak_count++;
            }
        }

        weak_ref<T>& operator=(const weak_ref& other) noexcept {
            destroy_this();

            block = other.block;
            object_pointer = other.object_pointer;

            if (block != nullptr) {
                block->weak_count++;
            }

            return *this;
        }

        weak_ref(weak_ref&& other) noexcept
            : block(other.block), object_pointer(other.object_pointer) {
            other.block = nullptr;
            other.object_pointer = nullptr;
        }

        weak_ref<T>& operator=(weak_ref&& other) noexcept {
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
            if (block == nullptr) {
                return 0u;
            }

            return block->ref_count;
        }

        bool expired() const noexcept {
            if (block == nullptr) {
                return true;
            }

            return block->ref_count == 0u;
        }

        shared_ref<T> lock() {
            shared_ref<T> ref;

            if (!expired()) {
                ref.block = block;
                ref.object_pointer = object_pointer;

                // Increment the reference count, as we just created a new strong reference
                block->ref_count++;
            }

            return ref;
        }
    private:
        void destroy_this() noexcept {
            if (block == nullptr) {
                return;
            }

            block->weak_count--;
        }

        internal::ControlBlock* block {nullptr};
        T* object_pointer {nullptr};  // This is always null or something
    };
}
