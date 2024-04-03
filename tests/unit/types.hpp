#pragma once

#include <iostream>

struct S {
    S() {
        std::cout << "S()" << '\n';
    }

    ~S() {
        std::cout << "~S()" << '\n';
    }

    S(const S&) {
        std::cout << "S(const S&)" << '\n';
    }

    S& operator=(const S&) {
        std::cout << "S& operator=(const S&)" << '\n';

        return *this;
    }

    S(S&&) {
        std::cout << "S(S&&)" << '\n';
    }

    S& operator=(S&&) {
        std::cout << "S& operator=(S&&)" << '\n';

        return *this;
    }
};

struct Base {
    virtual ~Base() = default;

    virtual int x() const {
        return 21;
    }
};

struct Derived : Base {
    int x() const override {
        return 30;
    }
};

struct Derived2 : Base {
    int x() const override {
        return 52;
    }
};

struct Foo {
    int bar() const {
        return 21;
    }

    int bar() {
        return 30;
    }
};