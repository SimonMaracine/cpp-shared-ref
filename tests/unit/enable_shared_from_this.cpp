#include <gtest/gtest.h>
#include <cpp_shared_ref/memory.hpp>

struct SharingBase : public sm::enable_shared_from_this<SharingBase> {
    SharingBase(int foo)
        : foo(foo) {}

    virtual ~SharingBase() = default;

    sm::shared_ref<SharingBase> make_new() {
        return shared_from_this();
    }

    sm::shared_ref<const SharingBase> make_new() const {
        return shared_from_this();
    }

    sm::weak_ref<SharingBase> make_new_weak() {
        return weak_from_this();
    }

    sm::weak_ref<const SharingBase> make_new_weak() const {
        return weak_from_this();
    }

    int foo {};
};

struct SharingDerived : SharingBase {
    SharingDerived()
        : SharingBase(30) {}
};

TEST(enable_shared_from_this, SharedFromThis_Constructor) {
    {
        sm::shared_ref<SharingBase> p {new SharingBase(21)};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<SharingDerived> p {new SharingDerived};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<SharingBase> p {new SharingDerived};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<const SharingBase> p {new SharingBase(21)};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<const SharingDerived> p {new SharingDerived};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<const SharingBase> p {new SharingDerived};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }
}

TEST(enable_shared_from_this, SharedFromThis_Reset) {
    {
        sm::shared_ref<SharingBase> p;
        p.reset(new SharingBase(21));

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<SharingDerived> p;
        p.reset(new SharingDerived);

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<SharingBase> p;
        p.reset(new SharingDerived);

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<const SharingBase> p;
        p.reset(new SharingBase(21));

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<const SharingDerived> p;
        p.reset(new SharingDerived);

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<const SharingBase> p;
        p.reset(new SharingDerived);

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }
}

TEST(enable_shared_from_this, SharedFromThis_MakeShared) {
    {
        sm::shared_ref<SharingBase> p {sm::make_shared<SharingBase>(21)};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<SharingDerived> p {sm::make_shared<SharingDerived>()};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<SharingBase> p {sm::make_shared<SharingDerived>()};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<const SharingBase> p {sm::make_shared<SharingBase>(21)};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<const SharingDerived> p {sm::make_shared<SharingDerived>()};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }

    {
        sm::shared_ref<const SharingBase> p {sm::make_shared<SharingDerived>()};

        auto p2 {p->make_new()};

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_EQ(p2.use_count(), 2);
        ASSERT_EQ(p->foo, p2->foo);
    }
}

TEST(enable_shared_from_this, WeakFromThis_Constructor) {
    {
        sm::shared_ref<SharingBase> p {new SharingBase(21)};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<SharingDerived> p {new SharingDerived};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<SharingBase> p {new SharingDerived};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<const SharingBase> p {new SharingBase(21)};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<const SharingDerived> p {new SharingDerived};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<const SharingBase> p {new SharingDerived};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }
}

TEST(enable_shared_from_this, WeakFromThis_Reset) {
    {
        sm::shared_ref<SharingBase> p;
        p.reset(new SharingBase(21));

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<SharingDerived> p;
        p.reset(new SharingDerived);

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<SharingBase> p;
        p.reset(new SharingDerived);

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<const SharingBase> p;
        p.reset(new SharingBase(21));

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<const SharingDerived> p;
        p.reset(new SharingDerived);

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<const SharingBase> p;
        p.reset(new SharingDerived);

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }
}

TEST(enable_shared_from_this, WeakFromThis_MakeShared) {
    {
        sm::shared_ref<SharingBase> p {sm::make_shared<SharingBase>(21)};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<SharingDerived> p {sm::make_shared<SharingDerived>()};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<SharingBase> p {sm::make_shared<SharingDerived>()};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<const SharingBase> p {sm::make_shared<SharingBase>(21)};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<const SharingDerived> p {sm::make_shared<SharingDerived>()};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }

    {
        sm::shared_ref<const SharingBase> p {sm::make_shared<SharingDerived>()};

        auto p2 {p->make_new_weak()};

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_EQ(p2.use_count(), 1);
        ASSERT_EQ(p->foo, p2.lock()->foo);
    }
}

TEST(enable_shared_from_this, Aliasing) {
    sm::shared_ref<SharingBase> p {sm::make_shared<SharingBase>(21)};

    sm::shared_ref<int> p2 {p, &p->foo};

    ASSERT_EQ(p.use_count(), 2);
    ASSERT_EQ(p2.use_count(), 2);
}
