#include <memory>

int main() {
#if 0
    std::shared_ptr<int> ptr {std::make_shared<int>(21)};
#else
    std::shared_ptr<int> ptr {new int(21)};
#endif
}
