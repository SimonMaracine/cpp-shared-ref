#include <string>
#include <cstring>
#include <utility>

#include <gtest/gtest.h>
#include <shared_ref/shared_ref.hpp>

#include "types.hpp"

TEST(shared_ref, NoAllocation) {
    sm::shared_ref<S> p;
    sm::shared_ref<S> p2 {p};
    sm::shared_ref<S> p3 = p;
    sm::shared_ref<S> p4 {nullptr};

    ASSERT_TRUE(!p);
    ASSERT_EQ(p.use_count(), 0u);
    ASSERT_EQ(p.get(), nullptr);

    ASSERT_TRUE(!p2);
    ASSERT_EQ(p2.use_count(), 0u);
    ASSERT_EQ(p2.get(), nullptr);

    ASSERT_TRUE(!p3);
    ASSERT_EQ(p3.use_count(), 0u);
    ASSERT_EQ(p3.get(), nullptr);

    ASSERT_TRUE(!p4);
    ASSERT_EQ(p4.use_count(), 0u);
    ASSERT_EQ(p4.get(), nullptr);
}

TEST(shared_ref, AllocationInt_Observers) {
    {
        sm::shared_ref<int> p;

        ASSERT_FALSE(p);
        ASSERT_EQ(p.use_count(), 0u);
        ASSERT_EQ(p.get(), nullptr);
    }

    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};

        ASSERT_TRUE(p);
        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_NE(p.get(), nullptr);
        ASSERT_EQ(*p, 21);
    }
}

TEST(shared_ref, AllocationString_Observers) {
    {
        sm::shared_ref<std::string> p;

        ASSERT_FALSE(p);
        ASSERT_EQ(p.use_count(), 0u);
        ASSERT_EQ(p.get(), nullptr);
    }

    {
        const char* STRING = "hello";

        sm::shared_ref<std::string> p {sm::make_shared<std::string>(STRING)};

        ASSERT_TRUE(p);
        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_NE(p.get(), nullptr);
        ASSERT_EQ(*p, std::string(STRING));
        ASSERT_EQ(p->size(), std::strlen(STRING));
    }

    {
        const char* STRING = "Hello, world! This string is not optimized, as it's too large.";

        sm::shared_ref<std::string> p {sm::make_shared<std::string>(STRING)};

        ASSERT_TRUE(p);
        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_NE(p.get(), nullptr);
        ASSERT_EQ(*p, std::string(STRING));
        ASSERT_EQ(p->size(), std::strlen(STRING));
    }
}

TEST(shared_ref, ReferenceCounting_Copy) {
    sm::shared_ref<int> p;
    p = sm::make_shared<int>(21);

    ASSERT_EQ(p.use_count(), 1u);

    {
        sm::shared_ref<int> p2 = p;

        ASSERT_EQ(p.use_count(), 2u);
        ASSERT_EQ(p2.use_count(), 2u);

        {
            sm::shared_ref<int> p3 {sm::make_shared<int>(30)};

            ASSERT_EQ(p3.use_count(), 1u);
            ASSERT_EQ(*p3, 30);

            p3 = p2;

            ASSERT_EQ(p.use_count(), 3u);
            ASSERT_EQ(p2.use_count(), 3u);
            ASSERT_EQ(p3.use_count(), 3u);
            ASSERT_EQ(*p3, 21);
        }

        ASSERT_EQ(p.use_count(), 2u);
        ASSERT_EQ(p2.use_count(), 2u);
    }

    ASSERT_EQ(p.use_count(), 1u);
}

TEST(shared_ref, ReferenceCounting_Move) {
    sm::shared_ref<int> p;
    p = sm::make_shared<int>(21);

    ASSERT_EQ(p.use_count(), 1u);

    {
        sm::shared_ref<int> p2 = p;

        ASSERT_EQ(p.use_count(), 2u);
        ASSERT_EQ(p2.use_count(), 2u);

        {
            sm::shared_ref<int> p3;
            p3 = p2;

            ASSERT_EQ(p.use_count(), 3u);
            ASSERT_EQ(p2.use_count(), 3u);
            ASSERT_EQ(p3.use_count(), 3u);

            sm::shared_ref<int> p4 {std::move(p2)};

            ASSERT_EQ(p.use_count(), 3u);
            ASSERT_EQ(p3.use_count(), 3u);
            ASSERT_EQ(p4.use_count(), 3u);

            {
                sm::shared_ref<int> p5 {sm::make_shared<int>(30)};

                ASSERT_EQ(p5.use_count(), 1u);
                ASSERT_EQ(*p5, 30);

                p5 = std::move(p4);

                ASSERT_EQ(p5.use_count(), 3u);
                ASSERT_EQ(*p5, 21);

                ASSERT_EQ(p.use_count(), 3u);
                ASSERT_EQ(p3.use_count(), 3u);
                ASSERT_EQ(p5.use_count(), 3u);
            }

            ASSERT_EQ(p.use_count(), 2u);
            ASSERT_EQ(p3.use_count(), 2u);
        }
    }

    ASSERT_EQ(p.use_count(), 1u);
}

TEST(shared_ref, AssignmentNullptr) {
    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};

        ASSERT_EQ(p.use_count(), 1u);

        p = nullptr;

        ASSERT_EQ(p.use_count(), 0u);
    }

    {
        sm::shared_ref<int> p;

        ASSERT_EQ(p.use_count(), 0u);

        p = nullptr;

        ASSERT_EQ(p.use_count(), 0u);
    }
}

TEST(shared_ref, ResetBare) {
    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};

        ASSERT_EQ(p.use_count(), 1u);

        p.reset();

        ASSERT_EQ(p.use_count(), 0u);
    }

    {
        sm::shared_ref<int> p;

        ASSERT_EQ(p.use_count(), 0u);

        p.reset();

        ASSERT_EQ(p.use_count(), 0u);
    }
}

TEST(shared_ref, ResetValue) {
    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};

        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_EQ(*p, 21);

        p.reset(new int(30));

        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_EQ(*p, 30);
    }

    {
        sm::shared_ref<int> p;

        ASSERT_EQ(p.use_count(), 0u);

        p.reset(new int(30));

        ASSERT_EQ(p.use_count(), 1u);
        ASSERT_EQ(*p, 30);
    }
}

TEST(shared_ref, ComparisonOperators) {
    sm::shared_ref<int> p;
    sm::shared_ref<int> p2 {sm::make_shared<int>(21)};
    sm::shared_ref<int> p3 {p2};
    sm::shared_ref<int> p4 {sm::make_shared<int>(21)};

    ASSERT_TRUE(p2 == p3);
    ASSERT_TRUE(p2 != p4);
    ASSERT_FALSE(p2 != p3);
    ASSERT_FALSE(p2 == p4);

    ASSERT_TRUE(p2 != nullptr);
    ASSERT_TRUE(p == nullptr);
    ASSERT_FALSE(p2 == nullptr);
    ASSERT_FALSE(p != nullptr);

    ASSERT_TRUE(nullptr != p2);
    ASSERT_TRUE(nullptr == p);
    ASSERT_FALSE(nullptr == p2);
    ASSERT_FALSE(nullptr != p);
}

TEST(shared_ref, Swap) {
    sm::shared_ref<int> p {sm::make_shared<int>(21)};
    sm::shared_ref<int> p2 {sm::make_shared<int>(30)};
    sm::shared_ref<int> p3 {p2};

    ASSERT_EQ(p.use_count(), 1u);
    ASSERT_EQ(*p, 21);
    ASSERT_EQ(p2.use_count(), 2u);
    ASSERT_EQ(*p2, 30);

    p.swap(p2);

    ASSERT_EQ(p.use_count(), 2u);
    ASSERT_EQ(*p, 30);
    ASSERT_EQ(p2.use_count(), 1u);
    ASSERT_EQ(*p2, 21);

    std::swap(p, p2);

    ASSERT_EQ(p.use_count(), 1u);
    ASSERT_EQ(*p, 21);
    ASSERT_EQ(p2.use_count(), 2u);
    ASSERT_EQ(*p2, 30);
}
