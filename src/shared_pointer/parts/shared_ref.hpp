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

        explicit SharedRef(T* object_pointer) {
            block = new internal::ControlBlock<T>;
            block->ref_count = 1;
            block->object.pointer = object_pointer;

            this->object_pointer = object_pointer;
        }

        ~SharedRef() {
            destroy_this();
        }

        // TODO polymorphism support

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
                return 0;
            }

            return block->ref_count;
        }
    private:
        explicit SharedRef(internal::ControlBlock<T>* block) noexcept
            : block(block), object_pointer(&block->object.value) {}

        void destroy_this() {
            if (block == nullptr) {
                return;
            }

            block->ref_count--;

            if (block->ref_count == 0) {
                delete block;  // TODO should delete control block itself only when there are no weak refs,
                                // otherwise only delete managed object
            }
        }

        internal::ControlBlock<T>* block = nullptr;
        T* object_pointer = nullptr;  // Direct access to object; this is always null or something

        template<typename U, typename... Args>
        friend SharedRef<U> make_shared(Args&&... args);

        template<typename U>
        friend class WeakRef;
    };

    template<typename T, typename... Args>
    SharedRef<T> make_shared(Args&&... args) {
        internal::ControlBlock<T>* block = new internal::ControlBlock<T>;
        block->ref_count = 1;
        new(&block->object.value) T(std::forward<Args>(args)...);

        return SharedRef<T>(block);
    }
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const sm::SharedRef<T>& pointer) {
    stream << pointer.get();

    return stream;
}

namespace std {
    template<typename T>
    struct hash<sm::SharedRef<T>> {
        size_t operator()(const sm::SharedRef<T>& pointer) const {
            return hash<T*>()(pointer.get());
        }
    };
}
