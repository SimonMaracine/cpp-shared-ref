#include <gtest/gtest.h>
#include <cpp_shared_ref/memory.hpp>

struct Foo : public sm::enable_shared_from_this<Foo> {
    Foo(int bar)
        : bar(bar) {}

    sm::shared_ref<Foo> make_new() {
        return shared_from_this();
    }

    int bar {};
};

TEST(enable_shared_from_this, SharedFromThis) {
    sm::shared_ref<Foo> p {sm::make_shared<Foo>(21)};

    auto p2 {p->make_new()};

    ASSERT_EQ(p.use_count(), 2u);
    ASSERT_EQ(p2.use_count(), 2u);
    ASSERT_EQ(p->bar, p2->bar);
}
