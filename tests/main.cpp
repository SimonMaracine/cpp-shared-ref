#include <string>
#include <cstring>
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

TEST(shared_ref, AllocationInt) {
    sm::shared_ref<int> ptr {sm::make_shared<int>(21)};

    ASSERT_TRUE(ptr);
    ASSERT_EQ(ptr.use_count(), 1u);
    ASSERT_NE(ptr.get(), nullptr);
    ASSERT_EQ(*ptr, 21);
}

TEST(shared_ref, AllocationString) {
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
