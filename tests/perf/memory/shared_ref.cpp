#include <shared_ref/shared_ref.hpp>

int main() {
#if 0
    sm::shared_ref<int> ptr {sm::make_shared<int>(21)};
#else
    sm::shared_ref<int> ptr {new int(21)};
#endif
}