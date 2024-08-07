#pragma once

#include <cstddef>
#include <utility>
#include <iosfwd>  // std::basic_ostream
#include <memory>  // std::unique_ptr, std::hash
#include <exception>
#include <type_traits>

#include "internal/control_block.hpp"

namespace sm {
    // Object thrown by the constructors of shared_ref that take weak_ref as the argument,
    // when the weak_ref refers to an already deleted object
    struct bad_weak_ref : public std::exception {
        bad_weak_ref() noexcept = default;
        bad_weak_ref(const bad_weak_ref&) noexcept = default;

        const char* what() const noexcept override {
            return "Shared pointer construction failed, as weak pointer manages no object";
        }
    };
}

namespace sm {
    template<typename T>
    class weak_ref;

    template<typename T>
    class enable_shared_from_this;

    // Smart pointer with reference-counting copy semantics
    template<typename T>
    class shared_ref {
    public:
        // Construct an empty shared_ref
        constexpr shared_ref() noexcept = default;

        // Construct an empty shared_ref
        constexpr shared_ref(std::nullptr_t) noexcept {}

        // Construct a shared_ref from an existing object created using new
        // If construction fails by a std::bad_alloc, the object is deleted
        template<typename U>
        explicit shared_ref(U* ptr)
            : m_ptr(ptr), m_block(ptr) {
            check_shared_from_this(ptr);
        }

        // Construct a shared_ref from an existing object not created using new
        // Destroy the object with this deleter
        // If construction fails by a std::bad_alloc, the object is deleted
        template<typename U, typename Deleter>
        shared_ref(U* ptr, Deleter deleter)
            : m_ptr(ptr), m_block(ptr, std::move(deleter)) {
            check_shared_from_this(ptr);
        }

        // Construct an empty shared_ref with this deleter
        template<typename Deleter>
        shared_ref(std::nullptr_t, Deleter deleter)
            : m_block(static_cast<T*>(nullptr), std::move(deleter)) {}

        // Aliasing constructor
        // Construct a shared_ref that shares ownership with another shared_ref, but stores a pointer to another object
        template<typename U>
        shared_ref(const shared_ref<U>& other, T* ptr) noexcept
            : m_ptr(ptr), m_block(other.m_block) {
            if (m_block) {
                m_block.strong_count()++;
            }
        }

        // Construct a shared_ref that shares ownership with a weak_ref
        // Throw an exception, if the weak_ref is empty
        template<typename U>
        explicit shared_ref(const weak_ref<U>& ref) {
            if (ref.expired()) {
                throw bad_weak_ref();
            }

            m_ptr = ref.m_ptr;
            m_block = ref.m_block;

            m_block.strong_count()++;
        }

        // Construct a shared_ref that takes ownership from a unique_ptr
        template<typename U, typename Deleter>
        shared_ref(std::unique_ptr<U, Deleter>&& ref)  // TODO what's up with Deleter being a reference type?
            : shared_ref(ref.release(), std::move(ref.get_deleter())) {}

        // Destroy this shared_ref object
        ~shared_ref() noexcept {
            destroy_this();
        }

        // Reset this shared_ref and transfer the ownership of the object managed by the unique_ptr to this
        template<typename U, typename Deleter>
        shared_ref& operator=(std::unique_ptr<U, Deleter>&& ref) {
            destroy_this();

            m_ptr = ref.release();
            m_block = internal::ControlBlock(m_ptr, std::move(ref.get_deleter()));

            return *this;
        }

        // Copy constructor
        // Construct a shared_ref that shares ownership with another shared_ref
        shared_ref(const shared_ref& other) noexcept
            : m_ptr(other.m_ptr), m_block(other.m_block) {
            if (m_block) {
                m_block.strong_count()++;
            }
        }

        // Copy constructor
        // Construct a shared_ref that shares ownership with another shared_ref
        template<typename U>
        shared_ref(const shared_ref<U>& other) noexcept
            : m_ptr(other.m_ptr), m_block(other.m_block) {
            if (m_block) {
                m_block.strong_count()++;
            }
        }

        // Copy assignment
        // Reset this shared_ref and instead share ownership with another shared_ref
        shared_ref& operator=(const shared_ref& other) noexcept {
            destroy_this();

            m_ptr = other.m_ptr;
            m_block = other.m_block;

            if (m_block) {
                m_block.strong_count()++;
            }

            return *this;
        }

        // Copy assignment
        // Reset this shared_ref and instead share ownership with another shared_ref
        template<typename U>
        shared_ref& operator=(const shared_ref<U>& other) noexcept {
            destroy_this();

            m_ptr = other.m_ptr;
            m_block = other.m_block;

            if (m_block) {
                m_block.strong_count()++;
            }

            return *this;
        }

        // Move constructor
        // Move-construct a shared_ref from another shared_ref
        shared_ref(shared_ref&& other) noexcept
            : m_ptr(other.m_ptr), m_block(other.m_block) {
            other.m_ptr = nullptr;
            other.m_block = {};
        }

        // Move constructor
        // Move-construct a shared_ref from another shared_ref
        template<typename U>
        shared_ref(shared_ref<U>&& other) noexcept
            : m_ptr(other.m_ptr), m_block(other.m_block) {
            other.m_ptr = nullptr;
            other.m_block = {};
        }

        // Move assignment
        // Reset this shared_ref and instead move another shared_ref into this
        shared_ref& operator=(shared_ref&& other) noexcept {
            destroy_this();

            m_ptr = other.m_ptr;
            m_block = other.m_block;

            other.m_ptr = nullptr;
            other.m_block = {};

            return *this;
        }

        // Move assignment
        // Reset this shared_ref and instead move another shared_ref into this
        template<typename U>
        shared_ref& operator=(shared_ref<U>&& other) noexcept {
            destroy_this();

            m_ptr = other.m_ptr;
            m_block = other.m_block;

            other.m_ptr = nullptr;
            other.m_block = {};

            return *this;
        }

        // Get the stored object pointer
        // Note that this might be different from the managed object
        T* get() const noexcept {
            return m_ptr;
        }

        // Get a reference to the stored object
        // Note that this might be different from the managed object
        T& operator*() const noexcept {
            return *m_ptr;
        }

        // Get the stored object pointer
        // Note that this might be different from the managed object
        T* operator->() const noexcept {
            return m_ptr;
        }

        // Get the reference count
        std::size_t use_count() const noexcept {
            if (!m_block) {
                return 0;
            }

            return m_block.strong_count();
        }

        // Check if the managed object has only one reference
        bool unique() const noexcept {
            return use_count() == 1;
        }

        // Check if the stored pointer is not null
        operator bool() const noexcept {
            return m_ptr != nullptr;
        }

        // Check if this shared_ref precedes the other
        template<typename U>
        bool owner_before(const shared_ref<U>& other) const noexcept {
            return m_block.base() < other.m_block.base();
        }

        // Check if this shared_ref precedes the weak_ref
        template<typename U>
        bool owner_before(const weak_ref<U>& other) const noexcept {
            return m_block.base() < other.m_block.base();
        }

        // Reset this shared_ref
        void reset() noexcept {
            destroy_this();

            m_ptr = nullptr;
            m_block = {};
        }

        // Reset this shared_ref and instead manage an existing object created using new
        // If construction fails by a std::bad_alloc, the object is deleted
        template<typename U>
        void reset(U* ptr) {
            destroy_this();

            m_ptr = ptr;
            m_block = internal::ControlBlock(ptr);

            check_shared_from_this(ptr);
        }

        // Reset this shared_ref and instead manage an existing object created using new
        // Destroy the object with this deleter
        // If construction fails by a std::bad_alloc, the object is deleted
        template<typename U, typename Deleter>
        void reset(U* ptr, Deleter deleter) {
            destroy_this();

            m_ptr = ptr;
            m_block = internal::ControlBlock(ptr, std::move(deleter));

            check_shared_from_this(ptr);
        }

        // Swap this shared_ref object with another one
        void swap(shared_ref& other) noexcept {
            std::swap(m_ptr, other.m_ptr);
            std::swap(m_block, other.m_block);
        }
    private:
        void destroy_this() noexcept {
            if (!m_block) {
                return;
            }

            if (--m_block.strong_count() == 0) {
                m_ptr = nullptr;
                m_block.destroy();

                if (--m_block.weak_count() == 0) {
                    m_block.dispose();
                }
            }
        }

        template<typename U, typename = void>
        struct has_sft_base : std::false_type {};

        template<typename U>
        struct has_sft_base<U, std::void_t<
            decltype(enable_shared_from_this_base(static_cast<U*>(nullptr)))
        >> : std::true_type {};

        template<typename U, typename U2 = std::remove_cv_t<U>>
        std::enable_if_t<has_sft_base<U2>::value>
        check_shared_from_this(U* ptr) noexcept {
            if (!m_ptr->weak_this.expired()) {
                return;
            }

            m_ptr->weak_this.assign(const_cast<U2*>(ptr), m_block);
        }

        template<typename U>
        std::enable_if_t<!has_sft_base<U>::value>
        check_shared_from_this(U*) noexcept {}

        T* m_ptr {nullptr};
        internal::ControlBlock m_block;

        template<typename U, typename... Args>
        friend shared_ref<U> make_shared(Args&&... args);

        template<typename Deleter, typename U>
        friend Deleter* get_deleter(const shared_ref<U>& ref) noexcept;

        template<typename U>
        friend class weak_ref;

        template<typename U>
        friend class shared_ref;
    };

    // Construct a new shared_ref using new, with these arguments
    template<typename T, typename... Args>
    shared_ref<T> make_shared(Args&&... args) {
        shared_ref<T> ref;
        ref.m_block = internal::ControlBlock(ref.m_ptr, internal::MakeSharedTag(), std::forward<Args>(args)...);
        ref.check_shared_from_this(ref.m_ptr);

        return ref;
    }

    // Safely static_cast this shared_ref to another shared_ref
    template<typename T, typename U>
    shared_ref<T> static_ref_cast(const shared_ref<U>& ref) noexcept {
        T* ptr {static_cast<T*>(ref.get())};

        return shared_ref<T>(ref, ptr);
    }

    // Safely dynamic_cast this shared_ref to another shared_ref
    template<typename T, typename U>
    shared_ref<T> dynamic_ref_cast(const shared_ref<U>& ref) noexcept {
        T* ptr {dynamic_cast<T*>(ref.get())};

        if (ptr == nullptr) {
            return shared_ref<T>();
        } else {
            return shared_ref<T>(ref, ptr);
        }
    }

    // Safely const_cast this shared_ref to another shared_ref
    template<typename T, typename U>
    shared_ref<T> const_ref_cast(const shared_ref<U>& ref) noexcept {
        T* ptr {const_cast<T*>(ref.get())};

        return shared_ref<T>(ref, ptr);
    }

    // Safely reinterpret_cast this shared_ref to another shared_ref
    template<typename T, typename U>
    shared_ref<T> reinterpret_ref_cast(const shared_ref<U>& ref) noexcept {
        T* ptr {reinterpret_cast<T*>(ref.get())};

        return shared_ref<T>(ref, ptr);
    }

    // Get a pointer to the deleter of the shared_ref object, or nullptr, if it doesn't have a custom deleter
    template<typename Deleter, typename T>
    Deleter* get_deleter(const shared_ref<T>& ref) noexcept {
        return static_cast<Deleter*>(ref.m_block.get_deleter(typeid(Deleter)));
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
    return lhs.get() < nullptr;
}

template<typename T>
bool operator<(std::nullptr_t, const sm::shared_ref<T>& rhs) noexcept {
    return nullptr < rhs.get();
}

template<typename T>
bool operator>(const sm::shared_ref<T>& lhs, std::nullptr_t) noexcept {
    return lhs.get() > nullptr;
}

template<typename T>
bool operator>(std::nullptr_t, const sm::shared_ref<T>& rhs) noexcept {
    return nullptr > rhs.get();
}

template<typename T>
bool operator<=(const sm::shared_ref<T>& lhs, std::nullptr_t) noexcept {
    return lhs.get() <= nullptr;
}

template<typename T>
bool operator<=(std::nullptr_t, const sm::shared_ref<T>& rhs) noexcept {
    return nullptr <= rhs.get();
}

template<typename T>
bool operator>=(const sm::shared_ref<T>& lhs, std::nullptr_t) noexcept {
    return lhs.get() >= nullptr;
}

template<typename T>
bool operator>=(std::nullptr_t, const sm::shared_ref<T>& rhs) noexcept {
    return nullptr > rhs.get();
}

// Write the shared_ref object to the output stream
template<typename CharType, typename Traits, typename T>
std::basic_ostream<CharType, Traits>& operator<<(std::basic_ostream<CharType, Traits>& stream, const sm::shared_ref<T>& ref) {
    stream << ref.get();

    return stream;
}

namespace std {
    // Swap two shared_ref objects
    template<typename T>
    void swap(sm::shared_ref<T>& lhs, sm::shared_ref<T>& rhs) noexcept {
        lhs.swap(rhs);
    }

    // Get the hash of the shared_ref object, i.e. the hash of the stored pointer
    template<typename T>
    struct hash<sm::shared_ref<T>> {
        size_t operator()(const sm::shared_ref<T>& ref) const noexcept {
            return hash<T*>()(ref.get());
        }
    };
}

namespace sm {
    // Smart pointer with reference-counting copy semantics, that doesn't keep the managed object alive
    template<typename T>
    class weak_ref {
    public:
        // Construct an empty weak_ref
        constexpr weak_ref() noexcept = default;

        // Construct a weak_ref that shares ownership with a shared_ref
        // Don't keep the managed object alive, if the last (strong) reference is destroyed
        weak_ref(const shared_ref<T>& ref) noexcept
            : m_ptr(ref.m_ptr), m_block(ref.m_block) {
            if (m_block) {
                m_block.weak_count()++;
            }
        }

        // Destroy this weak_ref object
        ~weak_ref() noexcept {
            destroy_this();
        }

        // Reset this weak_ref and instead share ownership with a shared_ref
        template<typename U>
        weak_ref& operator=(const shared_ref<U>& ref) noexcept {
            destroy_this();

            m_ptr = ref.m_ptr;
            m_block = ref.m_block;

            if (m_block) {
                m_block.weak_count()++;
            }

            return *this;
        }

        // Copy constructor
        // Construct a weak_ref that shares ownership with another weak_ref
        weak_ref(const weak_ref& other) noexcept
            : m_ptr(other.m_ptr), m_block(other.m_block) {
            if (m_block) {
                m_block.weak_count()++;
            }
        }

        // Copy constructor
        // Construct a weak_ref that shares ownership with another weak_ref
        template<typename U>
        weak_ref(const weak_ref<U>& other) noexcept
            : m_ptr(other.m_ptr), m_block(other.m_block) {
            if (m_block) {
                m_block.weak_count()++;
            }
        }

        // Copy assignment
        // Reset this weak_ref and instead share ownership with another weak_ref
        weak_ref<T>& operator=(const weak_ref& other) noexcept {
            destroy_this();

            m_ptr = other.m_ptr;
            m_block = other.m_block;

            if (m_block) {
                m_block.weak_count()++;
            }

            return *this;
        }

        // Copy assignment
        // Reset this weak_ref and instead share ownership with another weak_ref
        template<typename U>
        weak_ref<T>& operator=(const weak_ref<U>& other) noexcept {
            destroy_this();

            m_ptr = other.m_ptr;
            m_block = other.m_block;

            if (m_block) {
                m_block.weak_count()++;
            }

            return *this;
        }

        // Move constructor
        // Move-construct a weak_ref from another weak_ref
        weak_ref(weak_ref&& other) noexcept
            : m_ptr(other.m_ptr), m_block(other.m_block) {
            other.m_ptr = nullptr;
            other.m_block = {};
        }

        // Move constructor
        // Move-construct a weak_ref from another weak_ref
        template<typename U>
        weak_ref(weak_ref<U>&& other) noexcept
            : m_ptr(other.m_ptr), m_block(other.m_block) {
            other.m_ptr = nullptr;
            other.m_block = {};
        }

        // Move assignment
        // Reset this weak_ref and instead move another weak_ref into this
        weak_ref<T>& operator=(weak_ref&& other) noexcept {
            destroy_this();

            m_ptr = other.m_ptr;
            m_block = other.m_block;

            other.m_ptr = nullptr;
            other.m_block = {};

            return *this;
        }

        // Move assignment
        // Reset this weak_ref and instead move another weak_ref into this
        template<typename U>
        weak_ref<T>& operator=(weak_ref<U>&& other) noexcept {
            destroy_this();

            m_ptr = other.m_ptr;
            m_block = other.m_block;

            other.m_ptr = nullptr;
            other.m_block = {};

            return *this;
        }

        // Get the (strong) reference count
        std::size_t use_count() const noexcept {
            if (!m_block) {
                return 0;
            }

            return m_block.strong_count();
        }

        // Check if the managed object has been deleted
        bool expired() const noexcept {
            return use_count() == 0;
        }

        // Create a new shared_ref that shares ownership with this weak_ref object
        // Return an empty shared_ref, if the managed object has already expired
        shared_ref<T> lock() const noexcept {
            shared_ref<T> ref;

            if (!expired()) {
                ref.m_ptr = m_ptr;
                ref.m_block = m_block;

                ref.m_block.strong_count()++;
            }

            return ref;
        }

        // Check if this weak_ref precedes the other
        template<typename U>
        bool owner_before(const weak_ref<U>& other) const noexcept {
            return m_block.base() < other.m_block.base();
        }

        // Check if this weak_ref precedes the shared_ref
        template<typename U>
        bool owner_before(const shared_ref<U>& other) const noexcept {
            return m_block.base() < other.m_block.base();
        }

        // Reset this weak_ref
        void reset() noexcept {
            destroy_this();

            m_ptr = nullptr;
            m_block = {};
        }

        // Swap this weak_ref object with another one
        void swap(weak_ref& other) noexcept {
            std::swap(m_ptr, other.m_ptr);
            std::swap(m_block, other.m_block);
        }
    private:
        void destroy_this() noexcept {
            if (!m_block) {
                return;
            }

            if (--m_block.weak_count() == 0 && m_block.strong_count() == 0) {
                m_block.dispose();
            }
        }

        template<typename U>
        void assign(U* ptr, internal::ControlBlock block) noexcept {
            m_ptr = ptr;
            m_block = block;

            if (m_block) {
                m_block.weak_count()++;
            }
        }

        T* m_ptr {nullptr};
        internal::ControlBlock m_block;

        template<typename U>
        friend class shared_ref;

        template<typename U>
        friend class weak_ref;
    };
}

namespace std {
    // Swap two weak_ref objects
    template<typename T>
    void swap(sm::weak_ref<T>& lhs, sm::weak_ref<T>& rhs) noexcept {
        lhs.swap(rhs);
    }
}

namespace sm {
    // Functor that provides owner-based mixed-type ordering of shared_ref and weak_ref
    template<typename T = void>
    struct owner_less;

    template<typename T>
    struct owner_less<shared_ref<T>> {
        bool operator()(const shared_ref<T>& lhs, const shared_ref<T>& rhs) const noexcept {
            return lhs.owner_before(rhs);
        }

        bool operator()(const shared_ref<T>& lhs, const weak_ref<T>& rhs) const noexcept {
            return lhs.owner_before(rhs);
        }

        bool operator()(const weak_ref<T>& lhs, const shared_ref<T>& rhs) const noexcept {
            return lhs.owner_before(rhs);
        }
    };

    template<typename T>
    struct owner_less<weak_ref<T>> {
        bool operator()(const weak_ref<T>& lhs, const weak_ref<T>& rhs) const noexcept {
            return lhs.owner_before(rhs);
        }

        bool operator()(const shared_ref<T>& lhs, const weak_ref<T>& rhs) const noexcept {
            return lhs.owner_before(rhs);
        }

        bool operator()(const weak_ref<T>& lhs, const shared_ref<T>& rhs) const noexcept {
            return lhs.owner_before(rhs);
        }
    };

    template<>
    struct owner_less<void> {
        template<typename T, typename U>
        bool operator()(const shared_ref<T>& lhs, const shared_ref<U>& rhs) const noexcept {
            return lhs.owner_before(rhs);
        }

        template<typename T, typename U>
        bool operator()(const shared_ref<T>& lhs, const weak_ref<U>& rhs) const noexcept {
            return lhs.owner_before(rhs);
        }

        template<typename T, typename U>
        bool operator()(const weak_ref<T>& lhs, const shared_ref<U>& rhs) const noexcept {
            return lhs.owner_before(rhs);
        }

        template<typename T, typename U>
        bool operator()(const weak_ref<T>& lhs, const weak_ref<U>& rhs) const noexcept {
            return lhs.owner_before(rhs);
        }

        using is_transparent = void;
    };

    // Class that, when publicly inherited from, allows an object currently managed by shared_ref to safely create
    // new shared_ref instances
    // Calling shared_from_this on an object not currently managed by a shared_ref throws a bad_weak_ref object
    template<typename T>
    class enable_shared_from_this {
    public:
        // Return a new shared_ref that shares ownership with the shared_ref currently managing the object T
        shared_ref<T> shared_from_this() {
            return shared_ref<T>(weak_this);
        }

        // Return a new shared_ref that shares ownership with the shared_ref currently managing the object T
        shared_ref<const T> shared_from_this() const {
            return shared_ref<const T>(weak_this);
        }

        // Return a new weak_ref that shares ownership with the shared_ref currently managing the object T
        weak_ref<T> weak_from_this() noexcept {
            return weak_ref<T>(weak_this);
        }

        // Return a new weak_ref that shares ownership with the shared_ref currently managing the object T
        weak_ref<const T> weak_from_this() const noexcept {
            return weak_ref<const T>(weak_this);
        }
    protected:
        constexpr enable_shared_from_this() noexcept {}
        ~enable_shared_from_this() {}

        enable_shared_from_this(const enable_shared_from_this& other) noexcept {}
        enable_shared_from_this& operator=(const enable_shared_from_this& other) noexcept { return *this; }
    private:
        friend const enable_shared_from_this* enable_shared_from_this_base(const enable_shared_from_this* p) { return p; }

        mutable weak_ref<T> weak_this;

        template<typename U>
        friend class shared_ref;
    };
}
