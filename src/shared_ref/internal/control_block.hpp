#pragma once

#include <cstddef>
#include <utility>
#include <typeinfo>

namespace sm {
    namespace internal {
        template<typename T>
        struct DefaultDeleter {
            void operator()(T* ptr) const noexcept {
                delete ptr;
            }
        };

        struct ControlBlockBase {
            virtual ~ControlBlockBase() noexcept = default;
            virtual void destroy() const noexcept = 0;
            virtual void* get_deleter(const std::type_info& ti) noexcept = 0;

            std::size_t strong_count {1u};
            std::size_t weak_count {0u};
        };

        template<typename T, typename Deleter>
        class ControlBlock : public ControlBlockBase {
        public:
            ControlBlock(T* ptr, Deleter deleter)
                : ptr(ptr), deleter(std::move(deleter)) {}

            void destroy() const noexcept override {
                deleter(ptr);
            }

            void* get_deleter(const std::type_info& ti) noexcept override {
                if (ti == typeid(Deleter)) {
                    return &deleter;  // TODO GCC uses addressof
                } else {
                    return nullptr;
                }
            }
        private:
            T* ptr {nullptr};
            Deleter deleter;
        };

        struct ControlBlockWrapper {
            ControlBlockWrapper() noexcept = default;

            template<typename T>
            ControlBlockWrapper(T* ptr) {
                try {
                    base = new ControlBlock(ptr, DefaultDeleter<T>());
                } catch (...) {
                    delete ptr;
                }
            }

            template<typename T, typename Deleter>
            ControlBlockWrapper(T* ptr, Deleter deleter) {
                try {
                    base = new ControlBlock(ptr, std::move(deleter));  // Safe to move here
                } catch (...) {
                    deleter(ptr);
                }
            }

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
