#pragma once

#include <cstddef>
#include <utility>
#include <ostream>

#include "internal/control_block.hpp"

namespace sm {
    template<typename T>
    class weak_ref;

    template<typename T>
    class shared_ref {
    public:
        shared_ref() noexcept = default;
        shared_ref(std::nullptr_t) noexcept {}

        shared_ref(T* object_pointer)
            : object_pointer(object_pointer) {
            block = new internal::ControlBlock;
        }

        ~shared_ref() {
            destroy_this();
        }

        shared_ref(const shared_ref& other) noexcept
            : block(other.block), object_pointer(other.object_pointer) {

            if (block != nullptr) {
                block->ref_count++;
            }
        }

        shared_ref& operator=(const shared_ref& other) {
            destroy_this();

            block = other.block;
            object_pointer = other.object_pointer;

            if (block != nullptr) {
                block->ref_count++;
            }

            return *this;
        }

        shared_ref(shared_ref&& other) noexcept
            : block(other.block), object_pointer(other.object_pointer) {
            other.block = nullptr;
            other.object_pointer = nullptr;
        }

        shared_ref& operator=(shared_ref&& other) {
            destroy_this();

            block = other.block;
            object_pointer = other.object_pointer;

            other.block = nullptr;
            other.object_pointer = nullptr;

            return *this;
        }

        shared_ref& operator=(std::nullptr_t) {
            destroy_this();

            block = nullptr;
            object_pointer = nullptr;

            return *this;
        }

        T* operator->() const noexcept {
            return object_pointer;
        }

        T& operator*() const noexcept {
            return *object_pointer;
        }

        operator bool() const noexcept {
            return object_pointer != nullptr;
        }

        // TODO operator==
        // TODO swap

        void reset() {
            destroy_this();

            block = nullptr;
            object_pointer = nullptr;
        }

        T* get() const noexcept {
            return object_pointer;
        }

        std::size_t use_count() const noexcept {
            if (block == nullptr) {
                return 0u;
            }

            return block->ref_count;
        }
    private:
        void destroy_this() {
            if (block == nullptr) {
                return;
            }

            if (--block->ref_count == 0u) {
                delete object_pointer;
                object_pointer = nullptr;

                if (block->weak_count == 0u) {
                    delete block;
                }
            }
        }

        internal::ControlBlock* block {nullptr};
        T* object_pointer {nullptr};

        template<typename U, typename... Args>
        friend shared_ref<U> make_shared(Args&&... args);

        template<typename U>
        friend class weak_ref;
    };

    template<typename T, typename... Args>
    shared_ref<T> make_shared(Args&&... args) {
        shared_ref<T> ref;

        ref.block = new internal::ControlBlock;
        ref.object_pointer = new T(std::forward<Args>(args)...);

        return ref;
    }
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const sm::shared_ref<T>& shared_ref) {
    stream << shared_ref.get();

    return stream;
}

namespace std {
    template<typename T>
    struct hash<sm::shared_ref<T>> {
        size_t operator()(const sm::shared_ref<T>& shared_ref) const {
            return hash<T*>()(shared_ref.get());
        }
    };
}
