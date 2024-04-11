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

        // Basic constructors

        template<typename U>
        explicit shared_ref(U* ptr)
            : ptr(ptr), block(ptr) {}

        template<typename U, typename Deleter>
        shared_ref(U* ptr, Deleter deleter)
            : ptr(ptr), block(ptr, std::move(deleter)) {}

        template<typename Deleter>
        shared_ref(std::nullptr_t, Deleter deleter)
            : block(static_cast<T*>(nullptr), std::move(deleter)) {}

        // Aliasing constructor

        template<typename U>
        shared_ref(const shared_ref<U>& other, T* ptr) noexcept
            : ptr(ptr), block(other.block) {
            if (block.base) {
                block.base->strong_count++;
            }
        }

        ~shared_ref() noexcept {
            destroy_this();
        }

        shared_ref& operator=(std::nullptr_t) noexcept {
            destroy_this();

            ptr = nullptr;
            block.base = nullptr;

            return *this;
        }

        // Copy constructors

        shared_ref(const shared_ref& other) noexcept
            : ptr(other.ptr), block(other.block) {
            if (block) {
                block.base->strong_count++;
            }
        }

        template<typename U>
        shared_ref(const shared_ref<U>& other) noexcept
            : ptr(other.ptr), block(other.block) {
            if (block) {
                block.base->strong_count++;
            }
        }

        // Copy assignments

        shared_ref& operator=(const shared_ref& other) noexcept {
            destroy_this();

            ptr = other.ptr;
            block = other.block;

            if (block) {
                block.base->strong_count++;
            }

            return *this;
        }

        template<typename U>
        shared_ref& operator=(const shared_ref<U>& other) noexcept {
            destroy_this();

            ptr = other.ptr;
            block = other.block;

            if (block) {
                block.base->strong_count++;
            }

            return *this;
        }

        // Move constructors

        shared_ref(shared_ref&& other) noexcept
            : ptr(other.ptr), block(other.block) {
            other.ptr = nullptr;
            other.block.base = nullptr;
        }

        template<typename U>
        shared_ref(shared_ref<U>&& other) noexcept
            : ptr(other.ptr), block(other.block) {
            other.ptr = nullptr;
            other.block.base = nullptr;
        }

        // Move assignments

        shared_ref& operator=(shared_ref&& other) noexcept {
            destroy_this();

            ptr = other.ptr;
            block = other.block;

            other.ptr = nullptr;
            other.block.base = nullptr;

            return *this;
        }

        template<typename U>
        shared_ref& operator=(shared_ref<U>&& other) noexcept {
            destroy_this();

            ptr = other.ptr;
            block = other.block;

            other.ptr = nullptr;
            other.block.base = nullptr;

            return *this;
        }

        T* get() const noexcept {
            return ptr;
        }

        T& operator*() const noexcept {
            return *ptr;
        }

        T* operator->() const noexcept {
            return ptr;
        }

        std::size_t use_count() const noexcept {
            if (!block) {
                return 0u;
            }

            return block.base->strong_count;
        }

        bool unique() const noexcept {
            return use_count() == 1u;
        }

        operator bool() const noexcept {
            return ptr != nullptr;
        }

        void reset() noexcept {
            destroy_this();

            ptr = nullptr;
            block.base = nullptr;
        }

        template<typename U>
        void reset(U* ptr) {
            destroy_this();

            this->ptr = ptr;
            block = internal::ControlBlockWrapper(ptr);
        }

        template<typename U, typename Deleter>
        void reset(U* ptr, Deleter deleter) {
            destroy_this();

            this->ptr = ptr;
            block = internal::ControlBlockWrapper(ptr, std::move(deleter));
        }

        void swap(shared_ref& other) noexcept {
            std::swap(ptr, other.ptr);
            std::swap(block, other.block);
        }
    private:
        void destroy_this() noexcept {
            if (!block) {
                return;
            }

            // Need to reset the pointers when they are deleted

            if (--block.base->strong_count == 0u) {
                ptr = nullptr;
                block.base->destroy();

                if (block.base->weak_count == 0u) {
                    block.destroy();
                }
            }
        }

        T* ptr {nullptr};
        internal::ControlBlockWrapper block;

        template<typename U, typename... Args>
        friend shared_ref<U> make_shared(Args&&... args);

        template<typename Deleter, typename U>
        friend Deleter* get_deleter(const shared_ref<U>& ref) noexcept;

        template<typename U>
        friend class weak_ref;

        // Needed by polymorphism
        template<typename U>
        friend class shared_ref;
    };

    template<typename T, typename... Args>
    shared_ref<T> make_shared(Args&&... args) {
        shared_ref<T> ref;

        ref.ptr = new T(std::forward<Args>(args)...);
        ref.block = internal::ControlBlockWrapper(ref.ptr);

        return ref;
    }

    // Cast functions

    template<typename T, typename U>
    shared_ref<T> static_ref_cast(const shared_ref<U>& ref) noexcept {
        T* ptr {static_cast<T*>(ref.get())};

        return shared_ref<T>(ref, ptr);
    }

    template<typename T, typename U>
    shared_ref<T> dynamic_ref_cast(const shared_ref<U>& ref) noexcept {
        T* ptr {dynamic_cast<T*>(ref.get())};

        if (ptr == nullptr) {
            return shared_ref<T>();
        } else {
            return shared_ref<T>(ref, ptr);
        }
    }

    template<typename T, typename U>
    shared_ref<T> const_ref_cast(const shared_ref<U>& ref) noexcept {
        T* ptr {const_cast<T*>(ref.get())};

        return shared_ref<T>(ref, ptr);
    }

    template<typename T, typename U>
    shared_ref<T> reinterpret_ref_cast(const shared_ref<U>& ref) noexcept {
        T* ptr {reinterpret_cast<T*>(ref.get())};

        return shared_ref<T>(ref, ptr);
    }

    template<typename Deleter, typename T>
    Deleter* get_deleter(const shared_ref<T>& ref) noexcept {
        return static_cast<Deleter*>(ref.block.base->get_deleter(typeid(Deleter)));
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
