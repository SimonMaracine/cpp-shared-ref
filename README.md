# cpp-shared-ref

A `C++17` compatible, non-atomic reference-counting smart pointer, made for myself as a replacement for the
atomic `std::shared_ptr`.

`shared_ref` emulates most of `std::shared_ptr`'s API and functionality, with the most important difference being
that the reference increments and decrements are not atomic. The premise is that a reference-counting smart pointer
can be very useful in contexts outside of threading, in which atomicity is not needed, being just useless overhead.
This is why I tried making my own version of `std::shared_ptr`.

Right now, `shared_ref` doesn't fully conform with the `std::shared_ptr` specification of `C++17`. This is probably
an incomplete list of missing features from my version or differences between the two:

- Not allocator-aware
- No support for arrays
- No support for `shared_from_this`
- `make_shared` doesn't allocate object and control block at the same time
- Missing a few `shared_ref` and `weak_ref` constructors
- Missing `owner_before` methods
- No support for `-fno-exceptions`
- Deprecated features as of `C++17` are missing (for good)

The code is unit-tested. Almost every public functionality is tested in its own unit. Also Valgrind is
regularly used in development to check for memory bugs. Tested compilation on `GCC 13.2` and `MSVC 19.39`.

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
