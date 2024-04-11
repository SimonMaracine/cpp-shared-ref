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
            : ptr(ref.ptr), block(ref.block) {
            if (block) {
                block.base->weak_count++;
            }
        }

        ~weak_ref() noexcept {
            destroy_this();
        }

        weak_ref& operator=(std::nullptr_t) noexcept {
            destroy_this();

            ptr = nullptr;
            block.base = nullptr;

            return *this;
        }

        weak_ref(const weak_ref& other) noexcept
            : ptr(other.ptr), block(other.block) {
            if (block) {
                block.base->weak_count++;
            }
        }

        weak_ref<T>& operator=(const weak_ref& other) noexcept {
            destroy_this();

            ptr = other.ptr;
            block = other.block;

            if (block) {
                block.base->weak_count++;
            }

            return *this;
        }

        weak_ref(weak_ref&& other) noexcept
            : ptr(other.ptr), block(other.block) {
            other.ptr = nullptr;
            other.block.base = nullptr;
        }

        weak_ref<T>& operator=(weak_ref&& other) noexcept {
            destroy_this();

            ptr = other.ptr;
            block = other.block;

            other.ptr = nullptr;
            other.block.base = nullptr;

            return *this;
        }

        std::size_t use_count() const noexcept {
            if (!block) {
                return 0u;
            }

            return block.base->strong_count;
        }

        bool expired() const noexcept {
            return use_count() == 0u;
        }

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

        void reset() noexcept {
            destroy_this();

            ptr = nullptr;
            block.base = nullptr;
        }

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
        internal::ControlBlockWrapper block;
    };
}

namespace std {
    template<typename T>
    void swap(sm::weak_ref<T>& lhs, sm::weak_ref<T>& rhs) noexcept {
        lhs.swap(rhs);
    }
}
