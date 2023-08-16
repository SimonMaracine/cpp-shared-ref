#pragma once

#include <cstddef>
#include <utility>

namespace sm {
    namespace internal {
        template<typename T>
        struct ControlBlock {
            template<typename... Args>
            ControlBlock(Args&&... args)
                : object_value(std::forward<Args>(args)...) {}

            ~ControlBlock() = default;

            ControlBlock(const ControlBlock&) = default;
            ControlBlock& operator=(const ControlBlock&) = default;
            ControlBlock(ControlBlock&&) = default;
            ControlBlock& operator=(ControlBlock&&) = default;

            T object_value;
            std::size_t ref_count = 1;
            std::size_t weak_count = 0;
        };
    }
}
