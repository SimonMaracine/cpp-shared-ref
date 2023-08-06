#pragma once

#include <cstddef>

namespace sm {
    namespace internal {
        template<typename T>
        struct ControlBlock {
            union {
                T* pointer;
                T instance;
            } object;

            size_t ref_count = 0;
            size_t weak_count = 0;
        };
    }
}
