#pragma once

#include <cstddef>

namespace sm {
    namespace internal {
        template<typename T>
        struct DefaultDeleter {
            void operator()(T* ptr) const noexcept {
                delete ptr;
            }
        };

        template<typename T>
        struct DeleterBase {
            virtual ~DeleterBase() = default;
            virtual void destroy(T* ptr) const noexcept = 0;
        };

        template<typename T, typename Deleter>
        class SharedRefDeleter : public DeleterBase<T> {
        public:
            explicit SharedRefDeleter(Deleter deleter)
                : deleter(deleter) {}

            void destroy(T* ptr) const noexcept override {
                deleter(ptr);
            }
        private:
            Deleter deleter;
        };

        template<typename T>
        struct ControlBlock {
            ControlBlock() {
                del_base = new SharedRefDeleter<T, DefaultDeleter<T>>(DefaultDeleter<T>());
            }

            template<typename Deleter>
            explicit ControlBlock(Deleter deleter) {
                del_base = new SharedRefDeleter<T, Deleter>(deleter);
            }

            ~ControlBlock() noexcept {
                delete del_base;
            }

            void destroy(T* ptr) const noexcept {
                del_base->destroy(ptr);
            }

            std::size_t ref_count {1u};
            std::size_t weak_count {0u};

            DeleterBase<T>* del_base {nullptr};
        };
    }
}
