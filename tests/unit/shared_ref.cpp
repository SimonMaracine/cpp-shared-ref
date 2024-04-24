#include <string>
#include <cstring>
#include <utility>
#include <cstdlib>
#include <functional>

#include <gtest/gtest.h>
#include <cpp_shared_ref/memory.hpp>

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

static void destroy(int* i) {
    std::free(i);
}

TEST(shared_ref, CustomDeleter) {
    int* i {static_cast<int*>(std::malloc(sizeof(int)))};
    int* i2 {static_cast<int*>(std::malloc(sizeof(int)))};

    *i = 21;
    *i2 = 21;

    struct Del {
        void operator()(int* i) const noexcept {
            std::free(i);
        }
    };

    sm::shared_ref<int> p {i, Del()};
    sm::shared_ref<int> p2 {nullptr, [](int* i) { std::free(i); }};

    sm::shared_ref<int> p3 {new int(30)};

    p3.reset(i2, destroy);
}

TEST(shared_ref, Casts) {
    {
        sm::shared_ref<Derived> p {sm::make_shared<Derived>()};
        sm::shared_ref<Base> p2 {sm::static_ref_cast<Base>(p)};

        ASSERT_EQ(p->x(), 30);
        ASSERT_EQ(p2->x(), 30);

        ASSERT_EQ(p.use_count(), 2u);
        ASSERT_EQ(p2.use_count(), 2u);
    }

    {
        sm::shared_ref<Derived2> p {sm::make_shared<Derived2>()};
        sm::shared_ref<Base> p2 {sm::static_ref_cast<Base>(p)};

        sm::shared_ref<Derived> p3 {sm::dynamic_ref_cast<Derived>(p)};

        ASSERT_TRUE(p3 == nullptr);

        ASSERT_EQ(p.use_count(), 2u);
        ASSERT_EQ(p2.use_count(), 2u);
    }

    {
        sm::shared_ref<Derived2> p {sm::make_shared<Derived2>()};
        sm::shared_ref<Base> p2 {sm::static_ref_cast<Base>(p)};

        sm::shared_ref<Derived2> p3 {sm::dynamic_ref_cast<Derived2>(p)};

        ASSERT_EQ(p3->x(), 52);

        ASSERT_EQ(p.use_count(), 3u);
        ASSERT_EQ(p2.use_count(), 3u);
        ASSERT_EQ(p3.use_count(), 3u);
    }

    {
        sm::shared_ref<Foo> p {sm::make_shared<Foo>()};
        sm::shared_ref<const Foo> p2 {sm::const_ref_cast<const Foo>(p)};

        ASSERT_EQ(p->bar(), 30);
        ASSERT_EQ(p2->bar(), 21);

        ASSERT_EQ(p.use_count(), 2u);
        ASSERT_EQ(p2.use_count(), 2u);
    }
}

TEST(shared_ref, Polymorphism) {
    {
        sm::shared_ref<Base> p {sm::make_shared<Derived>()};

        ASSERT_EQ(p->x(), 30);
    }

    {
        sm::shared_ref<Base> p;
        p = sm::make_shared<Derived>();

        ASSERT_EQ(p->x(), 30);
    }

    {
        sm::shared_ref<Derived> p {sm::make_shared<Derived>()};
        sm::shared_ref<Base> p2 {std::move(p)};

        ASSERT_EQ(p2->x(), 30);
        ASSERT_EQ(p2.use_count(), 1u);
    }

    {
        sm::shared_ref<Derived> p {sm::make_shared<Derived>()};
        sm::shared_ref<Base> p2;
        p2 = std::move(p);

        ASSERT_EQ(p2->x(), 30);
        ASSERT_EQ(p2.use_count(), 1u);
    }

    {
        sm::shared_ref<Base> p {sm::make_shared<Derived>()};
        p.reset(new Derived2);

        ASSERT_EQ(p->x(), 52);
        ASSERT_EQ(p.use_count(), 1u);
    }
}

static void destroy2(int* i) {
    std::free(i);
}

TEST(shared_ref, GetDeleter) {
    {
        int* i {static_cast<int*>(std::malloc(sizeof(int)))};

        sm::shared_ref<int> p {i, destroy2};

        auto deleter {sm::get_deleter<void(*)(int*)>(p)};

        ASSERT_EQ(*deleter, destroy2);
    }

    {
        sm::shared_ref<int> p {sm::make_shared<int>(21)};

        auto deleter {sm::get_deleter<void(*)(int*)>(p)};

        ASSERT_EQ(deleter, nullptr);
    }
}

TEST(shared_ref, Unique) {
    sm::shared_ref<int> p {sm::make_shared<int>(21)};
    ASSERT_TRUE(p.unique());

    {
        sm::shared_ref<int> p2 {p};

        ASSERT_FALSE(p.unique());
        ASSERT_FALSE(p2.unique());
    }

    ASSERT_TRUE(p.unique());
}

TEST(shared_ref, AliasingConstructor) {
    sm::shared_ref<char> p;

    {
        sm::shared_ref<Foo> p2 {sm::make_shared<Foo>()};

        p = sm::shared_ref<char>(p2, &p2->c);

        ASSERT_EQ(p2->bar(), 30);
        ASSERT_EQ(*p, 'S');
    }

    ASSERT_TRUE(p);
    ASSERT_EQ(p.use_count(), 1u);
    ASSERT_EQ(*p, 'S');
}

TEST(shared_ref, Hash) {
    int* pi {new int(21)};

    const std::size_t hash {std::hash<int*>()(pi)};

    sm::shared_ref<int> p {pi};

    ASSERT_EQ(hash, std::hash<sm::shared_ref<int>>()(p));
}

TEST(shared_ref, MakeShared) {
    {
        int integer {21};

        sm::weak_ref<NeedsDeletion> p;

        {
            sm::shared_ref<NeedsDeletion> p2 {sm::make_shared<NeedsDeletion>(&integer)};
            p = p2;

            ASSERT_EQ(integer, 21);
        }

        ASSERT_EQ(integer, 0);
    }

    {
        sm::shared_ref<Raii> p {sm::make_shared<Raii>()};
        // Test with Valgrind
    }
}

TEST(shared_ref, IncompleteType) {
    sm::shared_ref<NonExisting> p;
}

TEST(shared_ref, ConstructorWeakRef) {
    {
        sm::shared_ref<int> p {new int(21)};
        sm::weak_ref<int> w {p};

        p.reset();

        ASSERT_THROW(
            {
                sm::shared_ref<int> p2 {w};
            },
            sm::bad_weak_ref
        );
    }

    {
        sm::shared_ref<int> p {new int(21)};
        sm::weak_ref<int> w {p};

        sm::shared_ref<int> p2 {w};

        ASSERT_EQ(p2.use_count(), 2u);
    }
}
