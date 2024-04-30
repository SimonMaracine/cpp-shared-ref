#include <gtest/gtest.h>
#include <cpp_shared_ref/memory.hpp>

#include "types.hpp"

struct Sharing : public sm::enable_shared_from_this<Sharing> {
    Sharing(int foo)
        : foo(foo) {}

    sm::shared_ref<Sharing> make_new() {
        return shared_from_this();
    }

    int foo {};
};

// TODO test const versions, test all shared_ref constructors, test weak_from_this, test enable_shared_from_this

TEST(enable_shared_from_this, SharedFromThis) {
    {
        sm::shared_ref<Sharing> p {sm::make_shared<Sharing>(21)};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2u);
        ASSERT_EQ(p2.use_count(), 2u);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<Sharing> p {new Sharing(21)};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2u);
        ASSERT_EQ(p2.use_count(), 2u);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        // sm::shared_ref<Sharing> p {new Sharing(21)};

        // auto p2 {p->make_new()};

        // ASSERT_EQ(p.use_count(), 2u);
        // ASSERT_EQ(p2.use_count(), 2u);
        // ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<Sharing> p;
        p.reset(new Sharing(21));

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2u);
        ASSERT_EQ(p2.use_count(), 2u);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        // sm::shared_ref<Sharing> p;
        // p.reset(new Sharing(21));

        // auto p2 {p->make_new()};

        // ASSERT_EQ(p.use_count(), 2u);
        // ASSERT_EQ(p2.use_count(), 2u);
        // ASSERT_EQ(p->foo, p2->foo);
    }
}

// TEST(enable_shared_from_this, WeakFromThis) {

// }

TEST(enable_shared_from_this, Aliasing) {
    sm::shared_ref<Sharing> p {sm::make_shared<Sharing>(21)};

    sm::shared_ref<int> p2 {p, &p->foo};

    ASSERT_EQ(p.use_count(), 2u);
    ASSERT_EQ(p2.use_count(), 2u);
}
