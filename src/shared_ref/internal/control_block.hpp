#pragma once

#include <cstddef>
#include <utility>

namespace sm {
    namespace internal {
        struct ControlBlock {
            std::size_t ref_count = 1;
            std::size_t weak_count = 0;
        };
    }
}
