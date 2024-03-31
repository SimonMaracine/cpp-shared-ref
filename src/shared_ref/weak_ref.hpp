#pragma once

#include <cstddef>
#include <utility>

#include "internal/control_block.hpp"
#include "shared_ref.hpp"

namespace sm {
    template<typename T>
    class weak_ref {
    public:
        constexpr weak_ref() noexcept = default;
        constexpr weak_ref(std::nullptr_t) noexcept {}

        weak_ref(const shared_ref<T>& ref) noexcept
            : block(ref.block), object_ptr(ref.object_ptr) {
            if (block != nullptr) {
                block->weak_count++;
            }
        }

        ~weak_ref() noexcept {
            destroy_this();
        }

        weak_ref& operator=(std::nullptr_t) noexcept {
            destroy_this();

            block = nullptr;
            object_ptr = nullptr;

            return *this;
        }

        weak_ref(const weak_ref& other) noexcept
            : block(other.block), object_ptr(other.object_ptr) {
            if (block != nullptr) {
                block->weak_count++;
            }
        }

        weak_ref<T>& operator=(const weak_ref& other) noexcept {
            destroy_this();

            block = other.block;
            object_ptr = other.object_ptr;

            if (block != nullptr) {
                block->weak_count++;
            }

            return *this;
        }

        weak_ref(weak_ref&& other) noexcept
            : block(other.block), object_ptr(other.object_ptr) {
            other.block = nullptr;
            other.object_ptr = nullptr;
        }

        weak_ref<T>& operator=(weak_ref&& other) noexcept {
            destroy_this();

            block = other.block;
            object_ptr = other.object_ptr;

            other.block = nullptr;
            other.object_ptr = nullptr;

            return *this;
        }

        std::size_t use_count() const noexcept {
            if (block == nullptr) {
                return 0u;
            }

            return block->ref_count;
        }

        bool expired() const noexcept {
            return use_count() == 0u;
        }

        shared_ref<T> lock() const noexcept {
            shared_ref<T> ref;

            if (!expired()) {
                ref.block = block;
                ref.object_ptr = object_ptr;

                // Increment the reference count, as we just created a new strong reference
                block->ref_count++;
            }

            return ref;
        }

        void reset() noexcept {
            destroy_this();

            block = nullptr;
            object_ptr = nullptr;
        }

        void swap(weak_ref& other) noexcept {
            std::swap(block, other.block);
            std::swap(object_ptr, other.object_ptr);
        }
    private:
        void destroy_this() noexcept {
            if (block == nullptr) {
                return;
            }

            block->weak_count--;
        }

        internal::ControlBlock<T>* block {nullptr};
        T* object_ptr {nullptr};
    };
}

namespace std {
    template<typename T>
    void swap(sm::weak_ref<T>& lhs, sm::weak_ref<T>& rhs) noexcept {
        lhs.swap(rhs);
    }
}
