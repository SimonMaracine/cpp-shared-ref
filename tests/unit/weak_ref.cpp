#include <utility>

#include <gtest/gtest.h>
#include <cpp_shared_ref/memory.hpp>

#include "types.hpp"

TEST(weak_ref, NoAllocation) {
    sm::weak_ref<S> w;
    sm::weak_ref<S> w2 {w};
    sm::weak_ref<S> w3 = w;
    sm::weak_ref<S> w4 {nullptr};

    ASSERT_EQ(w.use_count(), 0);
    ASSERT_EQ(w2.use_count(), 0);
    ASSERT_EQ(w3.use_count(), 0);
    ASSERT_EQ(w4.use_count(), 0);
}

TEST(weak_ref, ReferenceCounting_Copy) {
    sm::weak_ref<int> w;
    sm::weak_ref<int> we;

    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};
        w = p;
        we = w;

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(w.use_count(), 1);
        ASSERT_EQ(we.use_count(), 1);

        {
            sm::weak_ref<int> w2 {w};

            ASSERT_EQ(p.use_count(), 1);
            ASSERT_EQ(w.use_count(), 1);
            ASSERT_EQ(w2.use_count(), 1);

            sm::weak_ref<int> w3;
            w3 = p;

            ASSERT_EQ(p.use_count(), 1);
            ASSERT_EQ(w.use_count(), 1);
            ASSERT_EQ(w2.use_count(), 1);
            ASSERT_EQ(w3.use_count(), 1);
        }

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(w.use_count(), 1);
    }

    ASSERT_EQ(w.use_count(), 0);
    ASSERT_EQ(we.use_count(), 0);
}

TEST(weak_ref, ReferenceCounting_Move) {
    sm::weak_ref<int> w;

    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};
        w = p;
        sm::weak_ref<int> w2 {w};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(w.use_count(), 1);
        ASSERT_EQ(w2.use_count(), 1);

        {
            sm::shared_ref<int> p2 {p};

            ASSERT_EQ(p.use_count(), 2);
            ASSERT_EQ(w.use_count(), 2);
            ASSERT_EQ(w2.use_count(), 2);
            ASSERT_EQ(p2.use_count(), 2);
        }

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(w.use_count(), 1);
        ASSERT_EQ(w2.use_count(), 1);

        sm::weak_ref<int> w3 {std::move(w2)};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(w.use_count(), 1);
        ASSERT_EQ(w3.use_count(), 1);

        w3 = std::move(w);

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(w3.use_count(), 1);
    }

    ASSERT_EQ(w.use_count(), 0);
}

TEST(weak_ref, Lock) {
    sm::weak_ref<int> w;

    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};
        w = p;

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(w.use_count(), 1);

        sm::shared_ref<int> p2 {w.lock()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(w.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);

        ASSERT_EQ(*p2, 21);
    }

    ASSERT_EQ(w.use_count(), 0);
    ASSERT_TRUE(w.lock() == nullptr);
}

TEST(weak_ref, Expired) {
    sm::weak_ref<int> w;

    ASSERT_TRUE(w.expired());

    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};
        w = p;

        ASSERT_FALSE(w.expired());
    }

    ASSERT_TRUE(w.expired());
}

TEST(weak_ref, Reset) {
    {
        sm::weak_ref<int> w;

        ASSERT_EQ(w.use_count(), 0);

        w.reset();

        ASSERT_EQ(w.use_count(), 0);
    }

    {
        sm::weak_ref<int> w;
        {
            sm::shared_ref<int> p {sm::make_shared<int>(21)};
            w = p;

            ASSERT_EQ(w.use_count(), 1);

            w.reset();

            ASSERT_EQ(w.use_count(), 0);
            ASSERT_TRUE(w.lock() == nullptr);
        }
    }
}

TEST(weak_ref, Swap) {
    sm::shared_ref<int> p {sm::make_shared<int>(21)};
    sm::shared_ref<int> p2 {sm::make_shared<int>(30)};
    sm::shared_ref<int> p3 {p2};
    sm::weak_ref<int> w {p};
    sm::weak_ref<int> w2 {p3};

    ASSERT_EQ(w.use_count(), 1);
    ASSERT_EQ(*w.lock(), 21);
    ASSERT_EQ(w2.use_count(), 2);
    ASSERT_EQ(*w2.lock(), 30);

    w.swap(w2);

    ASSERT_EQ(w.use_count(), 2);
    ASSERT_EQ(*w.lock(), 30);
    ASSERT_EQ(w2.use_count(), 1);
    ASSERT_EQ(*w2.lock(), 21);

    std::swap(w, w2);

    ASSERT_EQ(w.use_count(), 1);
    ASSERT_EQ(*w.lock(), 21);
    ASSERT_EQ(w2.use_count(), 2);
    ASSERT_EQ(*w2.lock(), 30);
}

TEST(weak_ref, Polymorphism) {
    {
        sm::shared_ref<Derived> p {sm::make_shared<Derived>()};
        sm::weak_ref<Base> p2 {p};

        ASSERT_EQ(p2.lock()->x(), 30);
    }

    {
        sm::shared_ref<Derived> p {sm::make_shared<Derived>()};
        sm::weak_ref<Base> p2;
        p2 = p;

        ASSERT_EQ(p2.lock()->x(), 30);
    }

    {
        sm::weak_ref<Derived> p {sm::weak_ref<Derived>()};
        sm::weak_ref<Base> p2 {p};
    }

    {
        sm::weak_ref<Derived> p {sm::weak_ref<Derived>()};
        sm::weak_ref<Base> p2;
        p2 = p;
    }

    {
        sm::weak_ref<Base> p {sm::weak_ref<Derived>()};
    }

    {
        sm::weak_ref<Base> p;
        p = sm::weak_ref<Derived>();
    }
}

TEST(weak_ref, IncompleteType) {
    sm::weak_ref<NonExisting> p;
}

TEST(weak_ref, OwnerBefore) {
    {
        sm::shared_ref<Ints> p {sm::make_shared<Ints>(21, 30)};

        sm::shared_ref<int> p2 {p, &p->a};
        sm::shared_ref<int> p3 {p, &p->b};

        sm::weak_ref<int> w2 {p2};
        sm::weak_ref<int> w3 {p3};

        ASSERT_FALSE(w2.owner_before(w3));
        ASSERT_FALSE(w3.owner_before(w2));
        ASSERT_FALSE(w2.owner_before(p2));
    }

    {
        sm::shared_ref<int> p;

        sm::shared_ref<int> p2 {p};
        sm::shared_ref<int> p3 {p};

        sm::weak_ref<int> w2 {p2};
        sm::weak_ref<int> w3 {p3};

        ASSERT_FALSE(w2.owner_before(w3));
        ASSERT_FALSE(w3.owner_before(w2));
        ASSERT_FALSE(w2.owner_before(p2));
    }

    {
        sm::shared_ref<int> p {sm::make_shared<int>()};
        sm::shared_ref<int> p2 {sm::make_shared<int>()};

        sm::weak_ref<int> w {p};
        sm::weak_ref<int> w2 {p2};

        ASSERT_TRUE(w.owner_before(w2) || w2.owner_before(w));
    }
}
