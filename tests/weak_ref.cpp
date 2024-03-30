#include <string>
#include <cstring>
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

    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};
        w = p;

        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_EQ(w.use_count(), 1u);

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
}
