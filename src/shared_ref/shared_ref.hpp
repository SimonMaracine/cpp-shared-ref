#pragma once

#include <cstddef>
#include <utility>
#include <ostream>

#include "internal/control_block.hpp"

namespace sm {
    template<typename T>
    class WeakRef;

    template<typename T>
    class SharedRef {
    public:
        SharedRef() noexcept = default;
        SharedRef(std::nullptr_t) noexcept {}

        SharedRef(T* object_pointer)
            : object_pointer(object_pointer) {
            block = new internal::ControlBlock;
        }

        ~SharedRef() {
            destroy_this();
        }

        SharedRef(const SharedRef& other) noexcept
            : block(other.block), object_pointer(other.object_pointer) {

            if (block != nullptr) {
                block->ref_count++;
            }
        }

        SharedRef& operator=(const SharedRef& other) {
            destroy_this();

            block = other.block;
            object_pointer = other.object_pointer;

            if (block != nullptr) {
                block->ref_count++;
            }

            return *this;
        }

        SharedRef(SharedRef&& other) noexcept
            : block(other.block), object_pointer(other.object_pointer) {
            other.block = nullptr;
            other.object_pointer = nullptr;
        }

        SharedRef& operator=(SharedRef&& other) {
            destroy_this();

            block = other.block;
            object_pointer = other.object_pointer;

            other.block = nullptr;
            other.object_pointer = nullptr;

            return *this;
        }

        SharedRef& operator=(std::nullptr_t) {
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
        friend SharedRef<U> make_shared(Args&&... args);

        template<typename U>
        friend class WeakRef;
    };

    template<typename T, typename... Args>
    SharedRef<T> make_shared(Args&&... args) {
        SharedRef<T> ref;

        ref.block = new internal::ControlBlock;
        ref.object_pointer = new T(std::forward<Args>(args)...);

        return ref;
    }
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const sm::SharedRef<T>& shared_ref) {
    stream << shared_ref.get();

    return stream;
}

namespace std {
    template<typename T>
    struct hash<sm::SharedRef<T>> {
        size_t operator()(const sm::SharedRef<T>& shared_ref) const {
            return hash<T*>()(shared_ref.get());
        }
    };
}
