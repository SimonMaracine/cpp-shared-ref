#pragma once

#include <cstddef>

#include "internal/control_block.hpp"

namespace sm {
    template<typename T>
    class WeakRef {
    public:
        WeakRef() = default;
        WeakRef(SharedRef<T>& shared_pointer);
        ~WeakRef();

        WeakRef(const WeakRef& other) noexcept;
        WeakRef& operator=(const WeakRef& other) noexcept;
        WeakRef(WeakRef&& other) noexcept;
        WeakRef& operator=(WeakRef&& other) noexcept;

        void reset();

        size_t use_count() const;
        bool expired() const;
        SharedRef<T> lock();
    private:
        void destroy_this() noexcept;

        internal::ControlBlock<T>* block = nullptr;
        SharedRef<T>* shared_pointer = nullptr;
    };

    template<typename T>
    WeakRef<T>::WeakRef(SharedRef<T>& shared_pointer) {
        block = shared_pointer.block;
        this->shared_pointer = &shared_pointer;

        block->weak_count++;
    }

    template<typename T>
    WeakRef<T>::~WeakRef() {
        destroy_this();
    }

    template<typename T>
    WeakRef<T>::WeakRef(const WeakRef& other) noexcept
        : block(other.block), shared_pointer(other.shared_pointer) {
        block->weak_count++;
    }

    template<typename T>
    WeakRef<T>& WeakRef<T>::operator=(const WeakRef& other) noexcept {
        destroy_this();

        block = other.block;
        shared_pointer = other.shared_pointer;

        block->weak_count++;

        return *this;
    }

    template<typename T>
    WeakRef<T>::WeakRef(WeakRef&& other) noexcept
        : block(other.block), shared_pointer(other.shared_pointer) {
        other.block = nullptr;
        other.shared_pointer = nullptr;
    }

    template<typename T>
    WeakRef<T>& WeakRef<T>::operator=(WeakRef&& other) noexcept {
        destroy_this();

        block = other.block;
        shared_pointer = other.shared_pointer;

        other.block = nullptr;
        other.shared_pointer = nullptr;

        return *this;
    }

    template<typename T>
    void WeakRef<T>::reset() {
        destroy_this();

        block = nullptr;
        shared_pointer = nullptr;
    }

    template<typename T>
    size_t WeakRef<T>::use_count() const {
        return block->ref_count;
    }

    template<typename T>
    bool WeakRef<T>::expired() const {
        return block->ref_count == 0;
    }

    template<typename T>
    SharedRef<T> WeakRef<T>::lock() {
        SharedRef<T> strong_pointer;

        if (!expired()) {
            strong_pointer.block = block;
            strong_pointer.object = block->object.pointer;  // FIXME wrong

            block->ref_count++;
        }

        return strong_pointer;
    }

    template<typename T>
    void WeakRef<T>::destroy_this() noexcept {
        if (block == nullptr) {
            return;
        }

        block->weak_count--;
    }
}
