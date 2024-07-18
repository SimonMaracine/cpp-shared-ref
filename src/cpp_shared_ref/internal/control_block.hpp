#pragma once

#include <cstddef>
#include <utility>
#include <typeinfo>
#include <memory>  // std::addressof

namespace sm {
    namespace internal {
        struct ControlBlockBase {
            virtual ~ControlBlockBase() noexcept = default;
            virtual void destroy() const noexcept = 0;
            virtual void* get_deleter(const std::type_info& ti) noexcept = 0;

            std::size_t strong_count {1};
            std::size_t weak_count {1};
        };

        template<typename T, typename Deleter>
        class ControlBlockDeleter final : public ControlBlockBase {
        public:
            ControlBlockDeleter(T* ptr, Deleter deleter) noexcept
                : m_object_ptr(ptr), m_deleter(std::move(deleter)) {}

            void destroy() const noexcept override {
                m_deleter(m_object_ptr);
            }

            void* get_deleter(const std::type_info& ti) noexcept override {
                if (ti == typeid(Deleter)) {
                    return std::addressof(m_deleter);
                } else {
                    return nullptr;
                }
            }
        private:
            T* m_object_ptr;
            Deleter m_deleter;
        };

        template<typename T>
        class ControlBlockPtr final : public ControlBlockBase {
        public:
            explicit ControlBlockPtr(T* ptr) noexcept
                : m_object_ptr(ptr) {}

            void destroy() const noexcept override {
                delete m_object_ptr;
            }

            void* get_deleter(const std::type_info&) noexcept override {
                return nullptr;
            }
        private:
            T* m_object_ptr;
        };

        struct MakeSharedTag {};

        template<typename T>
        class ControlBlockInPlace final : public ControlBlockBase {
        public:
            template<typename... Args>
            ControlBlockInPlace(Args&&... args) {
                ::new (std::addressof(m_impl.object)) T(std::forward<Args>(args)...);
            }

            void destroy() const noexcept override {
                m_impl.object.~T();
            }

            void* get_deleter(const std::type_info&) noexcept override {
                return nullptr;
            }

            T* get_ptr() noexcept {
                return std::addressof(m_impl.object);
            }
        private:
            union Impl {
                Impl() {}
                ~Impl() {}

                T object;
            } m_impl;
        };

        class ControlBlock final {
        public:
            ControlBlock() noexcept = default;

            template<typename T, typename Deleter>
            ControlBlock(T* ptr, Deleter deleter) {
                try {
                    m_base = new ControlBlockDeleter(ptr, std::move(deleter));  // Safe to move here
                } catch (...) {
                    deleter(ptr);
                    throw;
                }
            }

            template<typename T>
            explicit ControlBlock(T* ptr) {
                try {
                    m_base = new ControlBlockPtr(ptr);
                } catch (...) {
                    delete ptr;
                    throw;
                }
            }

            template<typename T, typename... Args>
            ControlBlock(T*& ptr, MakeSharedTag, Args&&... args) {
                auto block {new ControlBlockInPlace<T>(std::forward<Args>(args)...)};
                ptr = block->get_ptr();
                m_base = block;
            }

            // Helper methods

            void destroy() const noexcept {
                m_base->destroy();
            }

            void* get_deleter(const std::type_info& ti) const noexcept {
                return m_base->get_deleter(ti);
            }

            void dispose() noexcept {
                delete m_base;
                m_base = nullptr;
            }

            std::size_t strong_count() const noexcept {
                return m_base->strong_count;
            }

            std::size_t weak_count() const noexcept {
                return m_base->weak_count;
            }

            std::size_t& strong_count() noexcept {
                return m_base->strong_count;
            }

            std::size_t& weak_count() noexcept {
                return m_base->weak_count;
            }

            operator bool() const noexcept {
                return m_base != nullptr;
            }

            const void* base() const noexcept {
                return m_base;
            }
        private:
            ControlBlockBase* m_base {nullptr};
        };
    }
}
