#include <cpp_shared_ref/memory.hpp>

int main() {
#if 1
    sm::shared_ref<int> ptr {sm::make_shared<int>(21)};
#else
    sm::shared_ref<int> ptr {new int(21)};
#endif
}
