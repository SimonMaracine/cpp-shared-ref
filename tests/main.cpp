#include <string>
#include <cstring>
#include <utility>
#include <vector>
#include <unordered_map>
#include <memory>

#include <shared_pointer/shared_pointer.hpp>
#include <gtest/gtest.h>

struct Base {
    Base(bool* base_c, bool* base_d)
        : base_c(base_c), base_d(base_d) {

        *base_c = !*base_c;
    }

    virtual ~Base() {
        *base_d = !*base_d;
    }

    bool* base_c = nullptr;
    bool* base_d = nullptr;
};

struct Derived : Base {
    Derived(bool* base_c, bool* base_d, bool* derived_c, bool* derived_d)
        : Base(base_c, base_d), derived_c(derived_c), derived_d(derived_d)  {
        *derived_c = !*derived_c;
    }

    virtual ~Derived() {
        *derived_d = !*derived_d;
    }

    bool* derived_c = nullptr;
    bool* derived_d = nullptr;
};

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

TEST(SharedRefTest, InsideVector) {
    {
        bool base_c = false;
        bool base_d = false;

        std::vector<sm::SharedRef<Base>> vec;

        vec.push_back(sm::make_shared<Base>(&base_c, &base_d));

        ASSERT_TRUE(base_c);

        vec.pop_back();

        ASSERT_TRUE(base_d);
    }

    {
        std::vector<sm::SharedRef<std::string>> vec;

        vec.push_back(sm::make_shared<std::string>("Hello"));
        vec.push_back(sm::make_shared<std::string>(","));
        vec.push_back(sm::make_shared<std::string>("world"));
        vec.push_back(sm::make_shared<std::string>("!"));
        vec.push_back(sm::make_shared<std::string>("What's"));
        vec.push_back(sm::make_shared<std::string>("up"));
        vec.push_back(sm::make_shared<std::string>("?"));

        vec.clear();
    }
}

TEST(SharedRefTest, InsideUnorderedMap) {
    {
        bool base_c = false;
        bool base_d = false;

        std::unordered_map<sm::SharedRef<Base>, int> map;

        auto ptr = sm::make_shared<Base>(&base_c, &base_d);
        map[ptr] = 1;

        ASSERT_TRUE(base_c);

        map.erase(ptr);
        ptr.reset();

        ASSERT_TRUE(base_d);
    }

    {
        std::unordered_map<sm::SharedRef<std::string>, int> map;

        map[sm::make_shared<std::string>("Hello")] = 19;
        map[sm::make_shared<std::string>(",")] = 21;
        map[sm::make_shared<std::string>("world")] = 23;
        map[sm::make_shared<std::string>("!")] = 25;
        map[sm::make_shared<std::string>("What's")] = 27;
        map[sm::make_shared<std::string>("up")] = 29;
        map[sm::make_shared<std::string>("?")] = 31;

        map.clear();
    }
}

TEST(SharedRefTest, Polymorhism) {
    {
        bool base_c = false;
        bool base_d = false;
        bool derived_c = false;
        bool derived_d = false;

        sm::SharedRef<Derived> ptr = sm::make_shared<Derived>(&base_c, &base_d, &derived_c, &derived_d);

        ASSERT_TRUE(base_c);
        ASSERT_TRUE(derived_c);

        ptr.reset();

        ASSERT_TRUE(base_d);
        ASSERT_TRUE(derived_d);
    }

    // Copy constructor
    {
        bool base_c = false;
        bool base_d = false;
        bool derived_c = false;
        bool derived_d = false;

        auto derived_ptr = sm::make_shared<Derived>(&base_c, &base_d, &derived_c, &derived_d);
        sm::SharedRef<Base> ptr = derived_ptr;
        [[maybe_unused]] volatile auto want_copy_constructor = derived_ptr.use_count();

        ASSERT_TRUE(base_c);
        ASSERT_TRUE(derived_c);
        ASSERT_EQ(derived_ptr.use_count(), 2);
        ASSERT_EQ(ptr.use_count(), 2);

        ptr.reset();

        ASSERT_FALSE(base_d);
        ASSERT_FALSE(derived_d);
        ASSERT_EQ(derived_ptr.use_count(), 1);

        derived_ptr.reset();

        ASSERT_TRUE(base_d);
        ASSERT_TRUE(derived_d);
    }

    {
        bool base_c = false;
        bool base_d = false;
        bool derived_c = false;
        bool derived_d = false;

        auto derived_ptr = sm::make_shared<Derived>(&base_c, &base_d, &derived_c, &derived_d);
        sm::SharedRef<Base> ptr;
        [[maybe_unused]] volatile auto want_copy_assignment = ptr.use_count();
        ptr = derived_ptr;
        [[maybe_unused]] volatile auto want_copy_assignment2 = derived_ptr.use_count();

        ASSERT_TRUE(base_c);
        ASSERT_TRUE(derived_c);

        ptr.reset();

        ASSERT_TRUE(base_d);
        ASSERT_TRUE(derived_d);
    }

    {
        bool base_c = false;
        bool base_d = false;
        bool derived_c = false;
        bool derived_d = false;

        auto derived_ptr = sm::make_shared<Derived>(&base_c, &base_d, &derived_c, &derived_d);
        sm::SharedRef<Base> ptr = std::move(derived_ptr);

        ASSERT_TRUE(base_c);
        ASSERT_TRUE(derived_c);
        ASSERT_EQ(ptr.use_count(), 1);

        ptr.reset();

        ASSERT_TRUE(base_d);
        ASSERT_TRUE(derived_d);
    }
}
