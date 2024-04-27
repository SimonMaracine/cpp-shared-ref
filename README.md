# cpp-shared-ref

A cross-platform, `C++17` compatible, non-atomic reference-counting smart pointer, made for myself as a replacement
for the atomic `std::shared_ptr`.

`shared_ref` emulates most of `std::shared_ptr`'s API and functionality, with the most important difference being
that the reference increments and decrements are not atomic. The premise is that a reference-counting smart pointer
can be very useful in contexts outside of multithreading, in which atomicity is not needed, being just useless
overhead. This is why I tried making my own version of `std::shared_ptr`.

Right now, `shared_ref` doesn't fully conform to the `std::shared_ptr` specification of `C++17`. This is probably
an incomplete list of missing features from my version:

- Not allocator-aware
- No support for arrays
- No support for `shared_from_this`
- Missing `owner_before` methods
- No support for `-fno-exceptions`
- Deprecated features as of `C++17` are missing (for good)

The code is unit-tested. Almost all public functionality is tested. Also Valgrind is regularly used in development
to check for memory bugs. Tested compilation on `GCC 13.2` and `MSVC 19.39`.

If you need a specific missing feature, or need this library to work on lower versions of C++, feel free to open
up an issue.

## Usage

With CMake, you can use this library as a submodule:

```txt
git submodule add https://github.com/SimonMaracine/cpp-shared-ref <path/to/submodule>
```

```cmake
add_subdirectory(<path/to/submodule>)
target_link_libraries(<target> PRIVATE cpp_shared_ref)
```

To build with tests:

```cmake
set(CPP_SHARED_REF_BUILD_TESTS ON)
```

## Example

```cpp
#include <iostream>
#include <cpp_shared_ref/memory.hpp>

int main() {
    sm::weak_ref<int> foo;

    {
        sm::shared_ref<int> bar {sm::make_shared<int>(5)};
        std::cout << *bar << '\n';

        foo = bar;
    }

    auto baz {foo.lock()};

    if (baz) {
        std::cout << "This is never printed\n";
    }
}
```
