#include <iostream>
#include <utility>

#include <shared_pointer/shared_pointer.hpp>

#define PRINT_INFO(pointer) \
    std::cout << #pointer ": ref - " << pointer.use_count() << ", data - " << *pointer << std::endl;

int main() {
    sm::SharedPtr<int> ptr1 = sm::make_shared<int>(20);

    PRINT_INFO(ptr1)

    {
        sm::SharedPtr<int> ptr2 = ptr1;

        PRINT_INFO(ptr1)
        PRINT_INFO(ptr2)

        sm::SharedPtr<int> ptr3 = std::move(ptr2);

        PRINT_INFO(ptr1)
        PRINT_INFO(ptr3)
    }

    PRINT_INFO(ptr1)
}
