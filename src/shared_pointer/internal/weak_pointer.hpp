#pragma once

#include "control_block.hpp"

namespace sm {
    template<typename T>
    class WeakPtr {
    public:
        WeakPtr() = default;
        ~WeakPtr();

        WeakPtr(const WeakPtr& other);
        WeakPtr& operator=(const WeakPtr& other);
        WeakPtr(WeakPtr&& other) noexcept;
        WeakPtr& operator=(WeakPtr&& other) noexcept;

        void reset();

        size_t use_count() const;
        bool expired();
        SharedPtr<T> lock();
    private:
        internal::ControlBlock<T>* block = nullptr;
        SharedPtr<T>* shared_pointer = nullptr;
    };

    template<typename T>
    WeakPtr<T>::~WeakPtr(){

    }

    template<typename T>
    WeakPtr<T>::WeakPtr(const WeakPtr& other) {

    }

    template<typename T>
    WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& other) {

    }

    template<typename T>
    WeakPtr<T>::WeakPtr(WeakPtr&& other) noexcept {

    }

    template<typename T>
    WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& other) noexcept {

    }

    template<typename T>
    void WeakPtr<T>::reset() {

    }

    template<typename T>
    size_t WeakPtr<T>::use_count() const {
        return {};
    }

    template<typename T>
    bool WeakPtr<T>::expired() {
        return {};
    }

    template<typename T>
    SharedPtr<T> WeakPtr<T>::lock() {

    }
}
