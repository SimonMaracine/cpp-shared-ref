#include <string>
#include <cstring>
#include <utility>

#include <shared_pointer/shared_pointer.hpp>
#include <gtest/gtest.h>

TEST(SharedRefTest, Declaration) {
    sm::SharedRef<int> ptr;
    sm::SharedRef<int> ptr2 = ptr;
    sm::SharedRef<int> ptr3 = nullptr;

    ASSERT_TRUE(!ptr);
    ASSERT_EQ(ptr.use_count(), 0);
    ASSERT_EQ(ptr.get(), nullptr);

    ASSERT_TRUE(!ptr2);
    ASSERT_EQ(ptr2.use_count(), 0);
    ASSERT_EQ(ptr2.get(), nullptr);

    ASSERT_TRUE(!ptr3);
    ASSERT_EQ(ptr3.use_count(), 0);
    ASSERT_EQ(ptr3.get(), nullptr);
}

TEST(SharedRefTest, AllocationInt) {
    {
        sm::SharedRef<int> ptr = sm::make_shared<int>(21);

        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr.use_count(), 1);
        ASSERT_NE(ptr.get(), nullptr);
        ASSERT_EQ(*ptr, 21);
    }

    {
        sm::SharedRef<int> ptr {new int(21)};

        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr.use_count(), 1);
        ASSERT_NE(ptr.get(), nullptr);
        ASSERT_EQ(*ptr, 21);
    }
}

TEST(SharedRefTest, AllocationString) {
    const char* STRING = "Hello, world! This string is not optimized, as it's too large.";

    {
        sm::SharedRef<std::string> ptr = sm::make_shared<std::string>(STRING);

        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr.use_count(), 1);
        ASSERT_NE(ptr.get(), nullptr);
        ASSERT_EQ(*ptr, std::string(STRING));
        ASSERT_EQ(ptr->size(), std::strlen(STRING));
    }

    {
        sm::SharedRef<std::string> ptr {new std::string(STRING)};

        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr.use_count(), 1);
        ASSERT_NE(ptr.get(), nullptr);
        ASSERT_EQ(*ptr, std::string(STRING));
        ASSERT_EQ(ptr->size(), std::strlen(STRING));
    }
}

TEST(SharedRefTest, ResetVarious) {
    {
        sm::SharedRef<int> ptr;
        ptr.reset();

        ASSERT_TRUE(!ptr);
        ASSERT_EQ(ptr.use_count(), 0);
        ASSERT_EQ(ptr.get(), nullptr);
    }

    {
        sm::SharedRef<int> ptr = sm::make_shared<int>(21);
        ptr.reset();

        ASSERT_TRUE(!ptr);
        ASSERT_EQ(ptr.use_count(), 0);
        ASSERT_EQ(ptr.get(), nullptr);
    }

    {
        sm::SharedRef<std::string> ptr = sm::make_shared<std::string>("Hello, world! What's up? The ceiling.");
        ptr.reset();

        ASSERT_TRUE(!ptr);
        ASSERT_EQ(ptr.use_count(), 0);
        ASSERT_EQ(ptr.get(), nullptr);
    }

    {
        sm::SharedRef<int> ptr = sm::make_shared<int>(21);
        ptr = nullptr;

        ASSERT_TRUE(!ptr);
        ASSERT_EQ(ptr.use_count(), 0);
        ASSERT_EQ(ptr.get(), nullptr);
    }
}

TEST(SharedRefTest, IncRefInt) {
    sm::SharedRef<int> ptr = sm::make_shared<int>(21);

    ASSERT_EQ(ptr.use_count(), 1);

    sm::SharedRef<int> ptr2 = ptr;

    ASSERT_EQ(ptr.use_count(), 2);
    ASSERT_EQ(ptr2.use_count(), 2);

    sm::SharedRef<int> ptr3 = ptr2;

    ASSERT_EQ(ptr.use_count(), 3);
    ASSERT_EQ(ptr2.use_count(), 3);
    ASSERT_EQ(ptr3.use_count(), 3);
}

TEST(SharedRefTest, IncDecRefInt) {
    sm::SharedRef<int> ptr = sm::make_shared<int>(21);

    {
        sm::SharedRef<int> ptr2 = ptr;

        ASSERT_EQ(ptr.use_count(), 2);
        ASSERT_EQ(ptr2.use_count(), 2);

        {
            sm::SharedRef<int> ptr3 = ptr2;
            sm::SharedRef<int> ptr4 = ptr2;

            ASSERT_EQ(ptr.use_count(), 4);
            ASSERT_EQ(ptr2.use_count(), 4);
            ASSERT_EQ(ptr3.use_count(), 4);
            ASSERT_EQ(ptr4.use_count(), 4);

            sm::SharedRef<int> ptr5 = sm::make_shared<int>(22);
            {
                ASSERT_EQ(ptr5.use_count(), 1);

                ptr5 = ptr2;

                ASSERT_EQ(ptr.use_count(), 5);
                ASSERT_EQ(ptr2.use_count(), 5);
                ASSERT_EQ(ptr3.use_count(), 5);
                ASSERT_EQ(ptr4.use_count(), 5);
                ASSERT_EQ(ptr5.use_count(), 5);
            }
        }

        ASSERT_EQ(ptr.use_count(), 2);
        ASSERT_EQ(ptr2.use_count(), 2);
    }

    ASSERT_EQ(ptr.use_count(), 1);
}

TEST(SharedRefTest, IncDecRefMoveInt) {
    sm::SharedRef<int> ptr = sm::make_shared<int>(21);

    ASSERT_EQ(*ptr, 21);

    {
        sm::SharedRef<int> ptr2 = ptr;

        ASSERT_EQ(ptr.use_count(), 2);
        ASSERT_EQ(ptr2.use_count(), 2);

        sm::SharedRef<int> ptr3 = std::move(ptr2);

        ASSERT_EQ(ptr.use_count(), 2);
        ASSERT_EQ(ptr3.use_count(), 2);

        sm::SharedRef<int> ptr4;
        {
            ptr4 = std::move(ptr3);

            ASSERT_EQ(ptr.use_count(), 2);
            ASSERT_EQ(ptr4.use_count(), 2);

            ptr4 = sm::make_shared<int>(22);

            ASSERT_EQ(ptr.use_count(), 1);
            ASSERT_EQ(ptr4.use_count(), 1);

            ptr = std::move(ptr4);

            ASSERT_EQ(ptr.use_count(), 1);
        }
    }

    ASSERT_EQ(ptr.use_count(), 1);
    ASSERT_EQ(*ptr, 22);
}

// void std_stuff() {
//     std::shared_ptr<int> ptr = std::make_shared<int>(21);

//     std::weak_ptr<int> weak = ptr;

//     std::cout << ptr.use_count() << std::endl;
// }

// #define PRINT_INFO(pointer)
//     std::cout << #pointer ": ref - " << pointer.use_count() << ", data - " << *pointer << std::endl;

// int main() {
//     std_stuff();

//     sm::SharedRef<int> ptr1 = sm::make_shared<int>(20);

//     PRINT_INFO(ptr1)

//     {
//         sm::SharedRef<int> ptr2 = ptr1;

//         PRINT_INFO(ptr1)
//         PRINT_INFO(ptr2)

//         sm::SharedRef<int> ptr3 = std::move(ptr2);

//         PRINT_INFO(ptr1)
//         PRINT_INFO(ptr3)

//         sm::SharedRef<int> ptr4 = ptr3;

//         PRINT_INFO(ptr1)
//         PRINT_INFO(ptr3)
//         PRINT_INFO(ptr4)

//         ptr3 = sm::make_shared<int>(21);

//         PRINT_INFO(ptr1)
//         PRINT_INFO(ptr3)
//         PRINT_INFO(ptr4)

//         sm::WeakRef<int> weak = ptr3;

//         PRINT_INFO(ptr3)
//     }

//     PRINT_INFO(ptr1)

//     {
//         std::unordered_map<sm::SharedRef<int>, int> map;
//         map[sm::make_shared<int>(19)] = 82763;
//     }
// }
