#pragma once

#include <memory>
#include <iostream>
#include <utility>
#include <cstddef>

namespace sm {
    template <typename T>
    class SharedPtr {
    public:
        SharedPtr() = default;
        explicit SharedPtr(T* object);
        ~SharedPtr();

        SharedPtr(const SharedPtr& other);
        SharedPtr& operator=(const SharedPtr& other);
        SharedPtr(SharedPtr&& other) noexcept;
        SharedPtr& operator=(SharedPtr&& other) noexcept;

        T& operator->() const;
        T& operator*() const;

        operator bool() const;

        void reset();

        T* get() const;
        size_t use_count() const;
    private:
        struct ControlBlock {
            union {
                T* pointer;
                T instance;
            } object;

            size_t ref_count = 0;

            // TODO weak count
        };

        SharedPtr(ControlBlock* block);
        void destroy_this_pointer() noexcept;

        ControlBlock* block = nullptr;
        T* object = nullptr;  // Direct access to object

        template<typename U, typename... Args>
        friend SharedPtr<U> make_shared(Args&&... args);
    };

    template <typename T>
    SharedPtr<T>::SharedPtr(T *object) {
        block = new ControlBlock;
        block->ref_count = 1;
        block->object.pointer = object;

        this->object = object;
    }

    template <typename T>
    SharedPtr<T>::~SharedPtr() {
        destroy_this_pointer();
    }

    template <typename T>
    SharedPtr<T>::SharedPtr(const SharedPtr& other)
        : block(other.block), object(other.object) {

        block->ref_count++;
    }

    template<typename T>
    SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other) {
        destroy_this_pointer();

        block = other.block;
        object = other.object;

        block->ref_count++;

        return *this;
    }

    template<typename T>
    SharedPtr<T>::SharedPtr(SharedPtr&& other) noexcept
        : block(other.block), object(other.object) {

        other.block = nullptr;
        other.object = nullptr;
    }

    template<typename T>
    SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) noexcept {
        destroy_this_pointer();

        block = other.block;
        object = other.object;

        other.block = nullptr;
        other.object = nullptr;

        return *this;
    }

    template<typename T>
    T& SharedPtr<T>::operator->() const {
        return *object;
    }

    template<typename T>
    T& SharedPtr<T>::operator*() const {
        return *object;
    }

    template<typename T>
    SharedPtr<T>::operator bool() const {
        return object != nullptr;
    }

    template<typename T>
    void SharedPtr<T>::reset() {
        destroy_this_pointer();

        object = nullptr;
        block = nullptr;
    }

    template<typename T>
    T* SharedPtr<T>::get() const {
        return object;
    }

    template<typename T>
    size_t SharedPtr<T>::use_count() const {
        return block->ref_count;
    }

    template<typename T>
    SharedPtr<T>::SharedPtr(ControlBlock* block)
        : block(block), object(&block->object.instance) {}

    template<typename T>
    void SharedPtr<T>::destroy_this_pointer() noexcept {
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
    SharedPtr<T> make_shared(Args&&... args) {
        typename SharedPtr<T>::ControlBlock* block = new typename SharedPtr<T>::ControlBlock;
        block->ref_count = 1;
        ::new(&block->object.instance) T(std::forward<Args>(args)...);

        return SharedPtr<T>(block);
    }
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const sm::SharedPtr<T>& pointer) {
    stream << pointer.get();

    return stream;
}

namespace std {
    template<typename T>
    struct hash<sm::SharedPtr<T>> {
        size_t operator()(const sm::SharedPtr<T>& pointer) const {
            return hash<T*> {}(pointer.get());
        }
    };
}
