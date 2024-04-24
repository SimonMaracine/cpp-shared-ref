#pragma once

#include <cstddef>
#include <utility>
#include <typeinfo>
#include <memory>  // std::addressof

namespace sm {
    namespace internal {
        struct ControlBlockBase {
            virtual ~ControlBlockBase() noexcept = default;
            virtual void dispose() const noexcept = 0;
            virtual void* get_deleter(const std::type_info& ti) noexcept = 0;

            std::size_t strong_count {1u};
            std::size_t weak_count {0u};
        };

        template<typename T, typename Deleter>
        class ControlBlockDeleter final : public ControlBlockBase {
        public:
            ControlBlockDeleter(T* ptr, Deleter deleter) noexcept
                : object_ptr(ptr), deleter(std::move(deleter)) {}

            void dispose() const noexcept override {
                deleter(object_ptr);
            }

            void* get_deleter(const std::type_info& ti) noexcept override {
                if (ti == typeid(Deleter)) {
                    return std::addressof(deleter);
                } else {
                    return nullptr;
                }
            }
        private:
            T* object_ptr;
            Deleter deleter;
        };

        template<typename T>
        class ControlBlockPtr final : public ControlBlockBase {
        public:
            explicit ControlBlockPtr(T* ptr) noexcept
                : object_ptr(ptr) {}

            void dispose() const noexcept override {
                delete object_ptr;
            }

            void* get_deleter(const std::type_info&) noexcept override {
                return nullptr;
            }
        private:
            T* object_ptr;
        };

        struct MakeSharedTag {};

        template<typename T>
        class ControlBlockInPlace final : public ControlBlockBase {
        public:
            template<typename... Args>
            ControlBlockInPlace(Args&&... args) {
                ::new (&impl.object) T(std::forward<Args>(args)...);
            }

            void dispose() const noexcept override {
                impl.object.~T();
            }

            void* get_deleter(const std::type_info&) noexcept override {
                return nullptr;
            }

            T* get_ptr() noexcept {
                return &impl.object;
            }
        private:
            union Impl {
                Impl() {}
                ~Impl() {}

                T object;
            } impl;
        };

        struct ControlBlock final {
            ControlBlock() noexcept = default;

            template<typename T, typename Deleter>
            ControlBlock(T* ptr, Deleter deleter) {
                try {
                    base = new ControlBlockDeleter(ptr, std::move(deleter));  // Safe to move here
                } catch (...) {
                    deleter(ptr);
                    throw;
                }
            }

            template<typename T>
            explicit ControlBlock(T* ptr) {
                try {
                    base = new ControlBlockPtr(ptr);
                } catch (...) {
                    delete ptr;
                    throw;
                }
            }

            template<typename T, typename... Args>
            ControlBlock(T*& ptr, MakeSharedTag, Args&&... args) {
                auto block {new ControlBlockInPlace<T>(std::forward<Args>(args)...)};
                ptr = block->get_ptr();
                base = block;
            }

            // Helper methods

            void destroy() noexcept {
                delete base;
                base = nullptr;
            }

            operator bool() const noexcept {
                return base != nullptr;
            }

            ControlBlockBase* base {nullptr};
        };
    }
}
