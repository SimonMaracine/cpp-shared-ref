#pragma once

#include <cstddef>
#include <utility>

namespace sm {
    namespace internal {
        template<typename T>
        struct ControlBlock {
            template<typename... Args>
            ControlBlock(Args&&... args)
                : object.value(std::forward<Args>(args)...) {}

            ~ControlBlock() = default;

            ControlBlock(const ControlBlock&) = default;
            ControlBlock& operator=(const ControlBlock&) = default;
            ControlBlock(ControlBlock&&) = default;
            ControlBlock& operator=(ControlBlock&&) = default;

            union {
                T value;
                T* pointer;
            } object;

            std::size_t ref_count = 0;
            std::size_t weak_count = 0;
        };
    }
}
