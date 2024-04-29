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

    char c {'S'};
};

class NeedsDeletion {
public:
    NeedsDeletion(int* p)
        : p(p) {}

    ~NeedsDeletion() {
        *p = 0;
    }
private:
    int* p {nullptr};
};

struct NonExisting;

class Raii {
public:
    Raii()
        : p(new int(21)) {}

    ~Raii() {
        delete p;
    }
private:
    int* p {nullptr};
};

struct Ints {
    Ints(int a, int b)
        : a(a), b(b) {}

    int a {};
    int b {};
};
