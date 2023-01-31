#pragma once

#include <memory>

#include <utility>
#include <cstddef>

namespace sm {
    template <typename T>
    class SharedPtr {
    public:
        SharedPtr() = default;
        SharedPtr(T* object);
        ~SharedPtr();

        SharedPtr(const SharedPtr& other);
        SharedPtr& operator=(const SharedPtr& other);
        SharedPtr(SharedPtr&& other);
        SharedPtr& operator=(SharedPtr&& other);

        T& operator->() const;
        T& operator*() const;

        operator bool() const;

        void reset();

        T* get() const;
        size_t use_count() const;
        bool unique() const;
    private:
        struct ControlBlock {
            union {
                T* ptr;
                T instance;
            } object;

            size_t ref_count = 0;

            // TODO weak count
        };

        SharedPtr(ControlBlock* block);
        void destroy_this_pointer();

        ControlBlock* block = nullptr;
        T* object = nullptr;

        template<typename U, typename... Args>
        friend SharedPtr<U> make_shared(Args&&... args);
    };

    template <typename T>
    SharedPtr<T>::SharedPtr(T *object) {
        // this->object = object;  // FIXME this
        block->ref_count = 1;
    }

    template <typename T>
    SharedPtr<T>::~SharedPtr() {
        destroy_this_pointer();
    }

    template <typename T>
    SharedPtr<T>::SharedPtr(const SharedPtr& other) {
        block = other.block;
        object = other.object;

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
    SharedPtr<T>::SharedPtr(SharedPtr&& other) {
        // TODO implement
    }

    template<typename T>
    SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) {
        // TODO implement
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
    bool SharedPtr<T>::unique() const {
        return block->ref_count == 1;
    }

    template<typename T>
    SharedPtr<T>::SharedPtr(ControlBlock* block)
        : block(block), object(&block->object.instance) {}

    template<typename T>
    void SharedPtr<T>::destroy_this_pointer() {
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
