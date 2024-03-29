#pragma once

#include <cstddef>

namespace sm {
    namespace internal {
        struct ControlBlock {
            std::size_t ref_count {1u};
            std::size_t weak_count {0u};
        };
    }
}
