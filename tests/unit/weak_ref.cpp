#include <utility>

#include <gtest/gtest.h>
#include <shared_ref/shared_ref.hpp>
#include <shared_ref/weak_ref.hpp>

#include "types.hpp"

TEST(weak_ref, NoAllocation) {
    sm::weak_ref<S> w;
    sm::weak_ref<S> w2 {w};
    sm::weak_ref<S> w3 = w;
    sm::weak_ref<S> w4 {nullptr};

    ASSERT_EQ(w.use_count(), 0u);
    ASSERT_EQ(w2.use_count(), 0u);
    ASSERT_EQ(w3.use_count(), 0u);
    ASSERT_EQ(w4.use_count(), 0u);
}

TEST(weak_ref, ReferenceCounting_Copy) {
    sm::weak_ref<int> w;
    sm::weak_ref<int> we;

    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};
        w = p;
        we = w;

        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_EQ(w.use_count(), 1u);
        ASSERT_EQ(we.use_count(), 1u);

        {
            sm::weak_ref<int> w2 {w};

            ASSERT_EQ(p.use_count(), 1u);
            ASSERT_EQ(w.use_count(), 1u);
            ASSERT_EQ(w2.use_count(), 1u);

            sm::weak_ref<int> w3;
            w3 = p;

            ASSERT_EQ(p.use_count(), 1u);
            ASSERT_EQ(w.use_count(), 1u);
            ASSERT_EQ(w2.use_count(), 1u);
            ASSERT_EQ(w3.use_count(), 1u);
        }

        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_EQ(w.use_count(), 1u);
    }

    ASSERT_EQ(w.use_count(), 0u);
    ASSERT_EQ(we.use_count(), 0u);
}

TEST(weak_ref, ReferenceCounting_Move) {
    sm::weak_ref<int> w;

    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};
        w = p;
        sm::weak_ref<int> w2 {w};

        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_EQ(w.use_count(), 1u);
        ASSERT_EQ(w2.use_count(), 1u);

        {
            sm::shared_ref<int> p2 {p};

            ASSERT_EQ(p.use_count(), 2u);
            ASSERT_EQ(w.use_count(), 2u);
            ASSERT_EQ(w2.use_count(), 2u);
            ASSERT_EQ(p2.use_count(), 2u);
        }

        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_EQ(w.use_count(), 1u);
        ASSERT_EQ(w2.use_count(), 1u);

        sm::weak_ref<int> w3 {std::move(w2)};

        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_EQ(w.use_count(), 1u);
        ASSERT_EQ(w3.use_count(), 1u);

        w3 = std::move(w);

        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_EQ(w3.use_count(), 1u);
    }

    ASSERT_EQ(w.use_count(), 0u);
}

TEST(weak_ref, Lock) {
    sm::weak_ref<int> w;

    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};
        w = p;

        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_EQ(w.use_count(), 1u);

        sm::shared_ref<int> p2 {w.lock()};

        ASSERT_EQ(p.use_count(), 2u);
        ASSERT_EQ(w.use_count(), 2u);
        ASSERT_EQ(p2.use_count(), 2u);

        ASSERT_EQ(*p2, 21);
    }

    ASSERT_EQ(w.use_count(), 0u);
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

TEST(weak_ref, AssignmentNullptr) {
    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};
        sm::weak_ref<int> w {p};

        ASSERT_EQ(w.use_count(), 1u);

        w = nullptr;

        ASSERT_EQ(w.use_count(), 0u);
    }

    {
        sm::weak_ref<int> w;

        ASSERT_EQ(w.use_count(), 0u);

        w = nullptr;

        ASSERT_EQ(w.use_count(), 0u);
    }
}

TEST(weak_ref, Reset) {
    {
        sm::weak_ref<int> w;

        ASSERT_EQ(w.use_count(), 0u);

        w.reset();

        ASSERT_EQ(w.use_count(), 0u);
    }

    {
        sm::weak_ref<int> w;
        {
            sm::shared_ref<int> p {sm::make_shared<int>(21)};
            w = p;

            ASSERT_EQ(w.use_count(), 1u);

            w.reset();

            ASSERT_EQ(w.use_count(), 0u);
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

    ASSERT_EQ(w.use_count(), 1u);
    ASSERT_EQ(*w.lock(), 21);
    ASSERT_EQ(w2.use_count(), 2u);
    ASSERT_EQ(*w2.lock(), 30);

    w.swap(w2);

    ASSERT_EQ(w.use_count(), 2u);
    ASSERT_EQ(*w.lock(), 30);
    ASSERT_EQ(w2.use_count(), 1u);
    ASSERT_EQ(*w2.lock(), 21);

    std::swap(w, w2);

    ASSERT_EQ(w.use_count(), 1u);
    ASSERT_EQ(*w.lock(), 21);
    ASSERT_EQ(w2.use_count(), 2u);
    ASSERT_EQ(*w2.lock(), 30);
}

TEST(weak_ref, IncompleteType) {
    sm::weak_ref<NonExisting> p;
}
