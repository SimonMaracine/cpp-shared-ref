#pragma once

#include <cstddef>
#include <utility>

#include "internal/control_block.hpp"
#include "shared_ref.hpp"

namespace sm {
    // Smart pointer with reference-counting copy semantics, that doesn't keep the managed object alive
    template<typename T>
    class weak_ref {
    public:
        // Construct an empty weak_ref
        constexpr weak_ref() noexcept = default;

        // Construct an empty weak_ref
        constexpr weak_ref(std::nullptr_t) noexcept {}

        // Construct a weak_ref that shares ownership with a shared_ref
        // Don't keep the managed object alive, if the last (strong) reference is destroyed
        weak_ref(const shared_ref<T>& ref) noexcept
            : ptr(ref.ptr), block(ref.block) {
            if (block) {
                block.base->weak_count++;
            }
        }

        // Destroy this weak_ref object
        ~weak_ref() noexcept {
            destroy_this();
        }

        // Reset this weak_ref
        weak_ref& operator=(std::nullptr_t) noexcept {
            destroy_this();

            ptr = nullptr;
            block = {};

            return *this;
        }

        // Reset this weak_ref and instead share ownership with a shared_ref
        weak_ref& operator=(const shared_ref<T>& ref) noexcept {  // TODO polymorphism
            destroy_this();

            ptr = ref.ptr;
            block = ref.block;

            if (block) {
                block.base->weak_count++;
            }

            return *this;
        }

        // Copy constructor
        // Construct a weak_ref that shares ownership with another weak_ref
        weak_ref(const weak_ref& other) noexcept  // TODO polymorphism
            : ptr(other.ptr), block(other.block) {
            if (block) {
                block.base->weak_count++;
            }
        }

        // Copy assignment
        // Reset this weak_ref and instead share ownership with another weak_ref
        weak_ref<T>& operator=(const weak_ref& other) noexcept {
            destroy_this();

            ptr = other.ptr;
            block = other.block;

            if (block) {
                block.base->weak_count++;
            }

            return *this;
        }

        // Move constructor
        // Move-construct a weak_ref from another weak_ref
        weak_ref(weak_ref&& other) noexcept  // TODO polymorphism
            : ptr(other.ptr), block(other.block) {
            other.ptr = nullptr;
            other.block = {};
        }

        // Move assignment
        // Reset this weak_ref and instead and instead move another weak_ref into this
        weak_ref<T>& operator=(weak_ref&& other) noexcept {
            destroy_this();

            ptr = other.ptr;
            block = other.block;

            other.ptr = nullptr;
            other.block = {};

            return *this;
        }

        // Get the (strong) reference count
        std::size_t use_count() const noexcept {
            if (!block) {
                return 0u;
            }

            return block.base->strong_count;
        }

        // Check if the managed object has been deleted
        bool expired() const noexcept {
            return use_count() == 0u;
        }

        // Create a new shared_ref that shares ownership with this weak_ref object
        // Return an empty shared_ref, if the managed object has already expired
        shared_ref<T> lock() const noexcept {
            shared_ref<T> ref;

            if (!expired()) {
                ref.ptr = ptr;
                ref.block = block;

                // We just created a new strong reference
                block.base->strong_count++;
            }

            return ref;
        }

        // Reset this weak_ref
        void reset() noexcept {
            destroy_this();

            ptr = nullptr;
            block = {};
        }

        // Swap this weak_ref object with another one
        void swap(weak_ref& other) noexcept {
            std::swap(ptr, other.ptr);
            std::swap(block, other.block);
        }
    private:
        void destroy_this() noexcept {
            if (!block) {
                return;
            }

            if (--block.base->weak_count == 0u && block.base->strong_count == 0u) {
                block.destroy();
            }
        }

        T* ptr {nullptr};
        internal::ControlBlock block;
    };
}

namespace std {
    // Swap two weak_ref objects
    template<typename T>
    void swap(sm::weak_ref<T>& lhs, sm::weak_ref<T>& rhs) noexcept {
        lhs.swap(rhs);
    }
}
