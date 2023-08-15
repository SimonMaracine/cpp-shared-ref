#pragma once

#include <cstddef>

namespace sm {
    namespace internal {
        template<typename T>
        struct ControlBlock {
            struct Object {  // FIXME use union
                T* pointer;
                T value;
            } object;

            std::size_t ref_count = 0;
            std::size_t weak_count = 0;
        };
    }
}
