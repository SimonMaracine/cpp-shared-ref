#include <chrono>
#include <iostream>
#include <cstring>
#include <cstddef>
#include <memory>

#include <shared_ref/shared_ref.hpp>

enum class Type {
    Ref,
    Ptr
};

struct Obj {
    char c[64u] {};
};

template<typename T, typename SmartPointer, unsigned int Repeat>
double test_speed() {
    std::chrono::duration<double> total {0.0};

    for (unsigned int repeat {0u}; repeat < Repeat; repeat++) {
        const auto begin {std::chrono::high_resolution_clock::now()};

        SmartPointer p {new T};
        p->c[0u] = 21;

        for (unsigned int i {0u}; i < 1000u; i++) {
            static constexpr std::size_t POINTERS {20'000u};
            SmartPointer ps[POINTERS] {};

            for (std::size_t j {0u}; j < POINTERS; j++) {
                ps[j] = p;
            }
        }

        const auto end {std::chrono::high_resolution_clock::now()};

        total += end - begin;
    }

    return (
        static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(total).count())
        / static_cast<double>(Repeat)
    );
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Invalid arguments\n";
        return 1;
    }

    const char* arg {argv[1u]};
    Type type {};

    if (std::strcmp(arg, "ref") == 0) {
        type = Type::Ref;
    } else if (std::strcmp(arg, "ptr") == 0) {
        type = Type::Ptr;
    } else {
        std::cerr << "Invalid type\n";
        return 1;
    }

    double result {};

    switch (type) {
        case Type::Ref:
            result = test_speed<Obj, sm::shared_ref<Obj>, 100u>();
            break;
        case Type::Ptr:
            result = test_speed<Obj, std::shared_ptr<Obj>, 100u>();
            break;
    }

    std::cout << "Took " << result << " ms average; " << 100u << " iterations\n";
}
