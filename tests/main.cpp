#include <iostream>
#include <utility>
#include <unordered_map>
#include <memory>

#include <shared_pointer/shared_pointer.hpp>

void std_stuff() {
    std::shared_ptr<int> ptr = std::make_shared<int>(21);

    std::weak_ptr<int> weak = ptr;

    std::cout << ptr.use_count() << std::endl;
}

#define PRINT_INFO(pointer) \
    std::cout << #pointer ": ref - " << pointer.use_count() << ", data - " << *pointer << std::endl;

int main() {
    std_stuff();

    sm::SharedRef<int> ptr1 = sm::make_shared<int>(20);

    PRINT_INFO(ptr1)

    {
        sm::SharedRef<int> ptr2 = ptr1;

        PRINT_INFO(ptr1)
        PRINT_INFO(ptr2)

        sm::SharedRef<int> ptr3 = std::move(ptr2);

        PRINT_INFO(ptr1)
        PRINT_INFO(ptr3)

        sm::WeakRef<int> weak = ptr3;
    }

    PRINT_INFO(ptr1)

    {
        std::unordered_map<sm::SharedRef<int>, int> map;
        map[sm::make_shared<int>(19)] = 82763;
    }
}
