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
            : object_ptr(ptr) {
            block = new internal::ControlBlock(ptr);
        }

        template<typename Deleter>
        shared_ref(T* ptr, Deleter deleter)
            : object_ptr(ptr) {
            block = new internal::ControlBlock(ptr, deleter);
        }

        template<typename Deleter>
        shared_ref(std::nullptr_t, Deleter deleter) {
            block = new internal::ControlBlock(static_cast<T*>(nullptr), deleter);
        }

        // Constructor used by casts
        template<typename U>
        shared_ref(const shared_ref<U>& other, T* ptr) noexcept
            : object_ptr(ptr), block(other.block) {
            if (block != nullptr) {
                block->strong_count++;
            }
        }

        ~shared_ref() noexcept {
            destroy_this();
        }

        shared_ref& operator=(std::nullptr_t) noexcept {
            destroy_this();

            object_ptr = nullptr;
            block = nullptr;

            return *this;
        }

        shared_ref(const shared_ref& other) noexcept
            : object_ptr(other.object_ptr), block(other.block) {
            if (block != nullptr) {
                block->strong_count++;
            }
        }

        shared_ref& operator=(const shared_ref& other) noexcept {
            destroy_this();

            object_ptr = other.object_ptr;
            block = other.block;

            if (block != nullptr) {
                block->strong_count++;
            }

            return *this;
        }

        shared_ref(shared_ref&& other) noexcept
            : object_ptr(other.object_ptr), block(other.block) {
            other.object_ptr = nullptr;
            other.block = nullptr;
        }

        shared_ref& operator=(shared_ref&& other) noexcept {
            destroy_this();

            object_ptr = other.object_ptr;
            block = other.block;

            other.object_ptr = nullptr;
            other.block = nullptr;

            return *this;
        }

        T* get() const noexcept {
            return object_ptr;
        }

        T& operator*() const noexcept {
            return *object_ptr;
        }

        T* operator->() const noexcept {
            return object_ptr;
        }

        std::size_t use_count() const noexcept {
            if (block == nullptr) {
                return 0u;
            }

            return block->strong_count;
        }

        operator bool() const noexcept {
            return object_ptr != nullptr;
        }

        void reset() noexcept {
            destroy_this();

            object_ptr = nullptr;
            block = nullptr;
        }

        void reset(T* ptr) {
            destroy_this();

            object_ptr = ptr;
            block = new internal::ControlBlock(ptr);
        }

        template<typename Deleter>
        void reset(T* ptr, Deleter deleter) {
            destroy_this();

            object_ptr = ptr;
            block = new internal::ControlBlock(ptr, deleter);
        }

        void swap(shared_ref& other) noexcept {
            std::swap(object_ptr, other.object_ptr);
            std::swap(block, other.block);
        }
    private:
        void destroy_this() noexcept {
            if (block == nullptr) {
                return;
            }

            // Need to reset the pointers when they are deleted

            if (--block->strong_count == 0u) {
                block->destroy();
                object_ptr = nullptr;

                if (block->weak_count == 0u) {
                    delete block;
                    block = nullptr;
                }
            }
        }

        T* object_ptr {nullptr};
        internal::ControlBlock* block {nullptr};

        template<typename U, typename... Args>
        friend shared_ref<U> make_shared(Args&&... args);

        template<typename U>
        friend class weak_ref;

        // This is needed by polymorphism
        template<typename U>
        friend class shared_ref;
    };

    template<typename T, typename... Args>
    shared_ref<T> make_shared(Args&&... args) {
        shared_ref<T> ref;

        ref.object_ptr = new T(std::forward<Args>(args)...);
        ref.block = new internal::ControlBlock(ref.object_ptr);

        return ref;
    }

    // Cast functions

    template<typename T, typename U>
    shared_ref<T> static_ref_cast(const shared_ref<U>& other) noexcept {
        T* ptr {static_cast<T*>(other.get())};

        return shared_ref<T>(other, ptr);
    }

    template<typename T, typename U>
    shared_ref<T> dynamic_ref_cast(const shared_ref<U>& other) noexcept {
        T* ptr {dynamic_cast<T*>(other.get())};

        if (ptr == nullptr) {
            return shared_ref<T>();
        } else {
            return shared_ref<T>(other, ptr);
        }
    }

    template<typename T, typename U>
    shared_ref<T> const_ref_cast(const shared_ref<U>& other) noexcept {
        T* ptr {const_cast<T*>(other.get())};

        return shared_ref<T>(other, ptr);
    }

    template<typename T, typename U>
    shared_ref<T> reinterpret_ref_cast(const shared_ref<U>& other) noexcept {
        T* ptr {reinterpret_cast<T*>(other.get())};

        return shared_ref<T>(other, ptr);
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
