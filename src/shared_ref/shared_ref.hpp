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

        shared_ref& operator=(std::nullptr_t) noexcept {
            destroy_this();

            block = nullptr;
            object_pointer = nullptr;

            return *this;
        }

        ~shared_ref() noexcept {
            destroy_this();
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

        T* operator->() const noexcept {
            return object_pointer;
        }

        T& operator*() const noexcept {
            return *object_pointer;
        }

        operator bool() const noexcept {
            return object_pointer != nullptr;
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

        void reset() noexcept {
            destroy_this();

            block = nullptr;
            object_pointer = nullptr;
        }

        void reset(T* object_pointer) {
            destroy_this();

            block = new internal::ControlBlock;
            this->object_pointer = object_pointer;
        }

        // TODO reset with deleter

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
                delete object_pointer;
                object_pointer = nullptr;

                if (block->weak_count == 0u) {
                    delete block;
                    block = nullptr;
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
std::ostream& operator<<(std::ostream& stream, const sm::shared_ref<T>& shared_ref) {
    stream << shared_ref.get();

    return stream;
}

namespace std {
    template<typename T>
    void swap(sm::shared_ref<T>& lhs, sm::shared_ref<T>& rhs) noexcept {
        lhs.swap(rhs);
    }

    template<typename T>
    struct hash<sm::shared_ref<T>> {
        size_t operator()(const sm::shared_ref<T>& shared_ref) const noexcept {
            return hash<T*>()(shared_ref.get());
        }
    };
}
