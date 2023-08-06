#pragma once

#include <ostream>
#include <utility>
#include <cstddef>

#include "internal/control_block.hpp"
// #include "weak_pointer.hpp"

namespace sm {
    template <typename T>
    class SharedRef {
    public:
        SharedRef() = default;
        explicit SharedRef(T* object);
        ~SharedRef();

        // TODO polymorphism support

        SharedRef(const SharedRef& other) noexcept;
        SharedRef& operator=(const SharedRef& other) noexcept;
        SharedRef(SharedRef&& other) noexcept;
        SharedRef& operator=(SharedRef&& other) noexcept;

        T& operator->() const;
        T& operator*() const;

        operator bool() const;

        void reset();

        T* get() const;
        size_t use_count() const;
    private:
        SharedRef(internal::ControlBlock<T>* block);
        void destroy_this() noexcept;

        internal::ControlBlock<T>* block = nullptr;
        T* object = nullptr;  // Direct access to object

        template<typename U, typename... Args>
        friend SharedRef<U> make_shared(Args&&... args);

        template<typename U>
        friend class WeakRef;
    };

    template <typename T>
    SharedRef<T>::SharedRef(T* object) {
        block = new internal::ControlBlock<T>;
        block->ref_count = 1;
        block->object.pointer = object;

        this->object = object;
    }

    template <typename T>
    SharedRef<T>::~SharedRef() {
        destroy_this();
    }

    template <typename T>
    SharedRef<T>::SharedRef(const SharedRef& other) noexcept
        : block(other.block), object(other.object) {
        block->ref_count++;
    }

    template<typename T>
    SharedRef<T>& SharedRef<T>::operator=(const SharedRef& other) noexcept {
        destroy_this();  // TODO not quite right

        block = other.block;
        object = other.object;

        block->ref_count++;

        return *this;
    }

    template<typename T>
    SharedRef<T>::SharedRef(SharedRef&& other) noexcept
        : block(other.block), object(other.object) {
        other.block = nullptr;
        other.object = nullptr;
    }

    template<typename T>
    SharedRef<T>& SharedRef<T>::operator=(SharedRef&& other) noexcept {
        destroy_this();

        block = other.block;
        object = other.object;

        other.block = nullptr;
        other.object = nullptr;

        return *this;
    }

    template<typename T>
    T& SharedRef<T>::operator->() const {
        return *object;
    }

    template<typename T>
    T& SharedRef<T>::operator*() const {
        return *object;
    }

    template<typename T>
    SharedRef<T>::operator bool() const {
        return object != nullptr;
    }

    template<typename T>
    void SharedRef<T>::reset() {
        destroy_this();

        object = nullptr;
        block = nullptr;
    }

    template<typename T>
    T* SharedRef<T>::get() const {
        return object;
    }

    template<typename T>
    size_t SharedRef<T>::use_count() const {
        return block->ref_count;
    }

    template<typename T>
    SharedRef<T>::SharedRef(internal::ControlBlock<T>* block)
        : block(block), object(&block->object.instance) {}

    template<typename T>
    void SharedRef<T>::destroy_this() noexcept {
        if (block == nullptr) {
            return;
        }

        block->ref_count--;

        if (block->ref_count == 0) {
            delete block;  // TODO should delete control block itself only when there are no weak refs,
                            // otherwise only delete managed object
        }
    }

    template<typename T, typename... Args>
    SharedRef<T> make_shared(Args&&... args) {
        internal::ControlBlock<T>* block = new internal::ControlBlock<T>;
        block->ref_count = 1;
        ::new(&block->object.instance) T(std::forward<Args>(args)...);

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
