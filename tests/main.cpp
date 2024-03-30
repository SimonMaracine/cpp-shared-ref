#include <string>
#include <cstring>
#include <utility>
#include <memory>

#include <gtest/gtest.h>
#include <shared_ref/shared_ref.hpp>

#include "types.hpp"

static void original_shared_ptr() {
    std::shared_ptr<S> ptr;
    std::shared_ptr<S> ptr2 {ptr};
}

TEST(shared_ref, NoAllocation) {
    {
        sm::shared_ref<S> ptr;
        sm::shared_ref<S> ptr2 {ptr};
        sm::shared_ref<S> ptr3 = ptr;

        ASSERT_TRUE(!ptr);
        ASSERT_EQ(ptr.use_count(), 0u);
        ASSERT_EQ(ptr.get(), nullptr);

        ASSERT_TRUE(!ptr2);
        ASSERT_EQ(ptr2.use_count(), 0u);
        ASSERT_EQ(ptr2.get(), nullptr);

        ASSERT_TRUE(!ptr3);
        ASSERT_EQ(ptr3.use_count(), 0u);
        ASSERT_EQ(ptr3.get(), nullptr);
    }

    {
        sm::shared_ref<S> ptr {nullptr};

        ASSERT_TRUE(!ptr);
        ASSERT_EQ(ptr.use_count(), 0u);
        ASSERT_EQ(ptr.get(), nullptr);
    }
}

TEST(shared_ref, AllocationInt_Observers) {
    {
        sm::shared_ref<int> ptr;

        ASSERT_FALSE(ptr);
        ASSERT_EQ(ptr.use_count(), 0u);
        ASSERT_EQ(ptr.get(), nullptr);
    }

    {
        sm::shared_ref<int> ptr {sm::make_shared<int>(21)};

        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr.use_count(), 1u);
        ASSERT_NE(ptr.get(), nullptr);
        ASSERT_EQ(*ptr, 21);
    }
}

TEST(shared_ref, AllocationString_Observers) {
    {
        sm::shared_ref<std::string> ptr;

        ASSERT_FALSE(ptr);
        ASSERT_EQ(ptr.use_count(), 0u);
        ASSERT_EQ(ptr.get(), nullptr);
    }

    {
        const char* STRING = "hello";

        sm::shared_ref<std::string> ptr {sm::make_shared<std::string>(STRING)};

        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr.use_count(), 1u);
        ASSERT_NE(ptr.get(), nullptr);
        ASSERT_EQ(*ptr, std::string(STRING));
        ASSERT_EQ(ptr->size(), std::strlen(STRING));
    }

    {
        const char* STRING = "Hello, world! This string is not optimized, as it's too large.";

        sm::shared_ref<std::string> ptr {sm::make_shared<std::string>(STRING)};

        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr.use_count(), 1u);
        ASSERT_NE(ptr.get(), nullptr);
        ASSERT_EQ(*ptr, std::string(STRING));
        ASSERT_EQ(ptr->size(), std::strlen(STRING));
    }
}

TEST(shared_ref, ReferenceCounting_Copy) {
    sm::shared_ref<int> ptr;
    ptr = sm::make_shared<int>(21);

    ASSERT_EQ(ptr.use_count(), 1u);

    {
        sm::shared_ref<int> ptr2 = ptr;

        ASSERT_EQ(ptr.use_count(), 2u);
        ASSERT_EQ(ptr2.use_count(), 2u);

        {
            sm::shared_ref<int> ptr3 {sm::make_shared<int>(30)};

            ASSERT_EQ(ptr3.use_count(), 1u);
            ASSERT_EQ(*ptr3, 30);

            ptr3 = ptr2;

            ASSERT_EQ(ptr.use_count(), 3u);
            ASSERT_EQ(ptr2.use_count(), 3u);
            ASSERT_EQ(ptr3.use_count(), 3u);
            ASSERT_EQ(*ptr3, 21);
        }

        ASSERT_EQ(ptr.use_count(), 2u);
        ASSERT_EQ(ptr2.use_count(), 2u);
    }

    ASSERT_EQ(ptr.use_count(), 1u);
}

TEST(shared_ref, ReferenceCounting_Move) {
    sm::shared_ref<int> ptr;
    ptr = sm::make_shared<int>(21);

    ASSERT_EQ(ptr.use_count(), 1u);

    {
        sm::shared_ref<int> ptr2 = ptr;

        ASSERT_EQ(ptr.use_count(), 2u);
        ASSERT_EQ(ptr2.use_count(), 2u);

        {
            sm::shared_ref<int> ptr3;
            ptr3 = ptr2;

            ASSERT_EQ(ptr.use_count(), 3u);
            ASSERT_EQ(ptr2.use_count(), 3u);
            ASSERT_EQ(ptr3.use_count(), 3u);

            sm::shared_ref<int> ptr4 {std::move(ptr2)};

            ASSERT_EQ(ptr.use_count(), 3u);
            ASSERT_EQ(ptr3.use_count(), 3u);
            ASSERT_EQ(ptr4.use_count(), 3u);

            {
                sm::shared_ref<int> ptr5 {sm::make_shared<int>(30)};

                ASSERT_EQ(ptr5.use_count(), 1u);
                ASSERT_EQ(*ptr5, 30);

                ptr5 = std::move(ptr4);

                ASSERT_EQ(ptr5.use_count(), 3u);
                ASSERT_EQ(*ptr5, 21);

                ASSERT_EQ(ptr.use_count(), 3u);
                ASSERT_EQ(ptr3.use_count(), 3u);
                ASSERT_EQ(ptr5.use_count(), 3u);
            }

            ASSERT_EQ(ptr.use_count(), 2u);
            ASSERT_EQ(ptr3.use_count(), 2u);
        }
    }

    ASSERT_EQ(ptr.use_count(), 1u);
}

TEST(shared_ptr, AssignmentNullptr) {
    {
        sm::shared_ref<int> ptr {sm::make_shared<int>(21)};

        ASSERT_EQ(ptr.use_count(), 1u);

        ptr = nullptr;

        ASSERT_EQ(ptr.use_count(), 0u);
    }

    {
        sm::shared_ref<int> ptr;

        ASSERT_EQ(ptr.use_count(), 0u);

        ptr = nullptr;

        ASSERT_EQ(ptr.use_count(), 0u);
    }
}

TEST(shared_ptr, ResetBare) {
    {
        sm::shared_ref<int> ptr {sm::make_shared<int>(21)};

        ASSERT_EQ(ptr.use_count(), 1u);

        ptr.reset();

        ASSERT_EQ(ptr.use_count(), 0u);
    }

    {
        sm::shared_ref<int> ptr;

        ASSERT_EQ(ptr.use_count(), 0u);

        ptr.reset();

        ASSERT_EQ(ptr.use_count(), 0u);
    }
}

TEST(shared_ptr, ResetValue) {
    {
        sm::shared_ref<int> ptr {sm::make_shared<int>(21)};

        ASSERT_EQ(ptr.use_count(), 1u);
        ASSERT_EQ(*ptr, 21);

        ptr.reset(new int(30));

        ASSERT_EQ(ptr.use_count(), 1u);
        ASSERT_EQ(*ptr, 30);
    }

    {
        sm::shared_ref<int> ptr;

        ASSERT_EQ(ptr.use_count(), 0u);

        ptr.reset(new int(30));

        ASSERT_EQ(ptr.use_count(), 1u);
        ASSERT_EQ(*ptr, 30);
    }
}

TEST(shared_ptr, ComparisonOperators) {
    sm::shared_ref<int> ptr;
    sm::shared_ref<int> ptr2 {sm::make_shared<int>(21)};
    sm::shared_ref<int> ptr3 {ptr2};
    sm::shared_ref<int> ptr4 {sm::make_shared<int>(21)};

    ASSERT_TRUE(ptr2 == ptr3);
    ASSERT_TRUE(ptr2 != ptr4);
    ASSERT_FALSE(ptr2 != ptr3);
    ASSERT_FALSE(ptr2 == ptr4);

    ASSERT_TRUE(ptr2 != nullptr);
    ASSERT_TRUE(ptr == nullptr);
    ASSERT_FALSE(ptr2 == nullptr);
    ASSERT_FALSE(ptr != nullptr);

    ASSERT_TRUE(nullptr != ptr2);
    ASSERT_TRUE(nullptr == ptr);
    ASSERT_FALSE(nullptr == ptr2);
    ASSERT_FALSE(nullptr != ptr);
}
