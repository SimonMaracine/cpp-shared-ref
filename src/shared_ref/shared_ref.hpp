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
        constexpr shared_ref() noexcept = default;
        constexpr shared_ref(std::nullptr_t) noexcept {}

        shared_ref(T* ptr)
            : object_pointer(ptr) {
            block = new internal::ControlBlock<T>;
        }

        template<typename Deleter>
        shared_ref(T* ptr, Deleter deleter)
            : object_pointer(ptr) {
            block = new internal::ControlBlock<T>(deleter);
        }

        template<typename Deleter>
        shared_ref(std::nullptr_t, Deleter deleter) {
            block = new internal::ControlBlock<T>(deleter);
        }

        ~shared_ref() noexcept {
            destroy_this();
        }

        shared_ref& operator=(std::nullptr_t) noexcept {
            destroy_this();

            block = nullptr;
            object_pointer = nullptr;

            return *this;
        }

        shared_ref(const shared_ref& other) noexcept
            : block(other.block), object_pointer(other.object_pointer) {

            if (block != nullptr) {
                block->ref_count++;
            }
        }

        shared_ref& operator=(const shared_ref& other) noexcept {
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

        shared_ref& operator=(shared_ref&& other) noexcept {
            destroy_this();

            block = other.block;
            object_pointer = other.object_pointer;

            other.block = nullptr;
            other.object_pointer = nullptr;

            return *this;
        }

        T* get() const noexcept {
            return object_pointer;
        }

        T& operator*() const noexcept {
            return *object_pointer;
        }

        T* operator->() const noexcept {
            return object_pointer;
        }

        std::size_t use_count() const noexcept {
            if (block == nullptr) {
                return 0u;
            }

            return block->ref_count;
        }

        operator bool() const noexcept {
            return object_pointer != nullptr;
        }

        void reset() noexcept {
            destroy_this();

            block = nullptr;
            object_pointer = nullptr;
        }

        void reset(T* ptr) {
            destroy_this();

            block = new internal::ControlBlock<T>;
            object_pointer = ptr;
        }

        template<typename Deleter>
        void reset(T* ptr, Deleter deleter) {
            destroy_this();

            block = new internal::ControlBlock<T>(deleter);
            object_pointer = ptr;
        }

        void swap(shared_ref& other) noexcept {
            std::swap(block, other.block);
            std::swap(object_pointer, other.object_pointer);
        }
    private:
        void destroy_this() noexcept {
            if (block == nullptr) {
                return;
            }

            // Need to reset the pointers when they are deleted

            if (--block->ref_count == 0u) {
                block->destroy(object_pointer);
                object_pointer = nullptr;

                if (block->weak_count == 0u) {
                    delete block;
                    block = nullptr;
                }
            }
        }

        internal::ControlBlock<T>* block {nullptr};
        T* object_pointer {nullptr};

        template<typename U, typename... Args>
        friend shared_ref<U> make_shared(Args&&... args);

        template<typename U>
        friend class weak_ref;
    };

    template<typename T, typename... Args>
    shared_ref<T> make_shared(Args&&... args) {
        shared_ref<T> ref;

        ref.block = new internal::ControlBlock<T>;
        ref.object_pointer = new T(std::forward<Args>(args)...);

        return ref;
    }
}

// Comparison operators with another shared_ref

template<typename T, typename U>
bool operator==(const sm::shared_ref<T>& lhs, const sm::shared_ref<U>& rhs) noexcept {
    return lhs.get() == rhs.get();
}

template<typename T, typename U>
bool operator!=(const sm::shared_ref<T>& lhs, const sm::shared_ref<U>& rhs) noexcept {
    return lhs.get() != rhs.get();
}

template<typename T, typename U>
bool operator<(const sm::shared_ref<T>& lhs, const sm::shared_ref<U>& rhs) noexcept {
    return lhs.get() < rhs.get();
}

template<typename T, typename U>
bool operator>(const sm::shared_ref<T>& lhs, const sm::shared_ref<U>& rhs) noexcept {
    return lhs.get() > rhs.get();
}

template<typename T, typename U>
bool operator<=(const sm::shared_ref<T>& lhs, const sm::shared_ref<U>& rhs) noexcept {
    return lhs.get() <= rhs.get();
}

template<typename T, typename U>
bool operator>=(const sm::shared_ref<T>& lhs, const sm::shared_ref<U>& rhs) noexcept {
    return lhs.get() >= rhs.get();
}

// Comparison operators with nullptr_t

template<typename T>
bool operator==(const sm::shared_ref<T>& lhs, std::nullptr_t) noexcept {
    return lhs.get() == nullptr;
}

template<typename T>
bool operator==(std::nullptr_t, const sm::shared_ref<T>& rhs) noexcept {
    return rhs.get() == nullptr;
}

template<typename T>
bool operator!=(const sm::shared_ref<T>& lhs, std::nullptr_t) noexcept {
    return lhs.get() != nullptr;
}

template<typename T>
bool operator!=(std::nullptr_t, const sm::shared_ref<T>& rhs) noexcept {
    return rhs.get() != nullptr;
}

template<typename T>
bool operator<(const sm::shared_ref<T>& lhs, std::nullptr_t) noexcept {
    return false;
}

template<typename T>
bool operator<(std::nullptr_t, const sm::shared_ref<T>& rhs) noexcept {
    return true;
}

template<typename T>
bool operator>(const sm::shared_ref<T>& lhs, std::nullptr_t) noexcept {
    return true;
}

template<typename T>
bool operator>(std::nullptr_t, const sm::shared_ref<T>& rhs) noexcept {
    return false;
}

template<typename T>
bool operator<=(const sm::shared_ref<T>& lhs, std::nullptr_t) noexcept {
    return false;
}

template<typename T>
bool operator<=(std::nullptr_t, const sm::shared_ref<T>& rhs) noexcept {
    return true;
}

template<typename T>
bool operator>=(const sm::shared_ref<T>& lhs, std::nullptr_t) noexcept {
    return true;
}

template<typename T>
bool operator>=(std::nullptr_t, const sm::shared_ref<T>& rhs) noexcept {
    return false;
}

template<typename T>
std::ostream& operator<<(std::ostream& stream, const sm::shared_ref<T>& ref) {
    stream << ref.get();

    return stream;
}

namespace std {
    template<typename T>
    void swap(sm::shared_ref<T>& lhs, sm::shared_ref<T>& rhs) noexcept {
        lhs.swap(rhs);
    }

    template<typename T>
    struct hash<sm::shared_ref<T>> {
        size_t operator()(const sm::shared_ref<T>& ref) const noexcept {
            return hash<T*>()(ref.get());
        }
    };
}
