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

            std::size_t strong_count {1u};
            std::size_t weak_count {0u};
        };

        template<typename T, typename Deleter>
        class ControlBlockDeleter : public ControlBlockBase {
        public:
            ControlBlockDeleter(T* ptr, Deleter deleter) noexcept
                : object_ptr(ptr), deleter(std::move(deleter)) {}

            void destroy() const noexcept override {
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
            T* object_ptr {nullptr};
            Deleter deleter;
        };

        template<typename T>
        class ControlBlock : public ControlBlockBase {
        public:
            ControlBlock(T* ptr) noexcept
                : object_ptr(ptr) {}

            void destroy() const noexcept override {
                delete object_ptr;
            }

            void* get_deleter(const std::type_info&) noexcept override {
                return nullptr;
            }
        private:
            T* object_ptr {nullptr};
        };

        struct ControlBlockWrapper {
            ControlBlockWrapper() noexcept = default;

            template<typename T>
            ControlBlockWrapper(T* ptr) {
                try {
                    base = new ControlBlock(ptr);
                } catch (...) {
                    delete ptr;
                    throw;
                }
            }

            template<typename T, typename Deleter>
            ControlBlockWrapper(T* ptr, Deleter deleter) {
                try {
                    base = new ControlBlockDeleter(ptr, std::move(deleter));  // Safe to move here
                } catch (...) {
                    deleter(ptr);
                    throw;
                }
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
