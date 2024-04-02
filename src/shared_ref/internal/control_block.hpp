#pragma once

#include <cstddef>
#include <typeinfo>

namespace sm {
    namespace internal {
        template<typename T>
        struct DefaultDeleter {
            void operator()(T* ptr) const noexcept {
                delete ptr;
            }
        };

        struct DeleterBase {
            virtual ~DeleterBase() noexcept = default;
            virtual void destroy() const noexcept = 0;
            virtual void* get_deleter(const std::type_info& ti) noexcept = 0;
        };

        template<typename T, typename Deleter>
        class SharedRefDeleter : public DeleterBase {
        public:
            SharedRefDeleter(T* ptr, Deleter deleter)
                : ptr(ptr), deleter(deleter) {}

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

        struct ControlBlock {
            template<typename T>
            ControlBlock(T* ptr) {
                deleter_base = new SharedRefDeleter(ptr, DefaultDeleter<T>());
            }

            template<typename T, typename Deleter>
            ControlBlock(T* ptr, Deleter deleter) {
                deleter_base = new SharedRefDeleter(ptr, deleter);
            }

            ~ControlBlock() noexcept {
                delete deleter_base;
            }

            void destroy() const noexcept {
                deleter_base->destroy();
            }

            std::size_t strong_count {1u};
            std::size_t weak_count {0u};

            DeleterBase* deleter_base {nullptr};
        };
    }
}
