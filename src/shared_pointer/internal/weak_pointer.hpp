#pragma once

#include "control_block.hpp"

namespace sm {
    template<typename T>
    class WeakPtr {
    public:
        WeakPtr() = default;
        WeakPtr(SharedPtr<T>& shared_pointer);
        ~WeakPtr();

        WeakPtr(const WeakPtr& other) noexcept;
        WeakPtr& operator=(const WeakPtr& other) noexcept;
        WeakPtr(WeakPtr&& other) noexcept;
        WeakPtr& operator=(WeakPtr&& other) noexcept;

        void reset();

        size_t use_count() const;
        bool expired() const;
        SharedPtr<T> lock();
    private:
        void destroy_this_pointer() noexcept;

        internal::ControlBlock<T>* block = nullptr;
        SharedPtr<T>* shared_pointer = nullptr;
    };

    template<typename T>
    WeakPtr<T>::WeakPtr(SharedPtr<T>& shared_pointer) {
        block = shared_pointer.block;
        this->shared_pointer = &shared_pointer;

        block->weak_count++;
    }

    template<typename T>
    WeakPtr<T>::~WeakPtr() {
        destroy_this_pointer();
    }

    template<typename T>
    WeakPtr<T>::WeakPtr(const WeakPtr& other) noexcept
        : block(other.block), shared_pointer(other.shared_pointer) {
        block->weak_count++;
    }

    template<typename T>
    WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& other) noexcept {
        destroy_this_pointer();

        block = other.block;
        shared_pointer = other.shared_pointer;

        block->weak_count++;

        return *this;
    }

    template<typename T>
    WeakPtr<T>::WeakPtr(WeakPtr&& other) noexcept
        : block(other.block), shared_pointer(other.shared_pointer) {
        other.block = nullptr;
        other.shared_pointer = nullptr;
    }

    template<typename T>
    WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& other) noexcept {
        destroy_this_pointer();

        block = other.block;
        shared_pointer = other.shared_pointer;

        other.block = nullptr;
        other.shared_pointer = nullptr;

        return *this;
    }

    template<typename T>
    void WeakPtr<T>::reset() {
        destroy_this_pointer();

        block = nullptr;
        shared_pointer = nullptr;
    }

    template<typename T>
    size_t WeakPtr<T>::use_count() const {
        return block->ref_count;
    }

    template<typename T>
    bool WeakPtr<T>::expired() const {
        return block->ref_count == 0;
    }

    template<typename T>
    SharedPtr<T> WeakPtr<T>::lock() {
        SharedPtr<T> strong_pointer;

        if (!expired()) {
            strong_pointer.block = block;
            strong_pointer.object = block->object.pointer;  // FIXME wrong

            block->ref_count++;
        }

        return strong_pointer;
    }

    template<typename T>
    void WeakPtr<T>::destroy_this_pointer() noexcept {
        if (block == nullptr) {
            return;
        }

        block->weak_count--;
    }
}
