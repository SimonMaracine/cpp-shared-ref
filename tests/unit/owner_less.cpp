#include <map>

#include <gtest/gtest.h>
#include <cpp_shared_ref/memory.hpp>

#include "types.hpp"

template<typename T>
static void SharedRef(T& map) {
    sm::shared_ref<Ints> p {sm::make_shared<Ints>(21, 30)};

    sm::shared_ref<int> p2 {p, &p->a};
    sm::shared_ref<int> p3 {p, &p->b};

    map[p2] = 52;
    map[sm::make_shared<int>(0)] = 0;

    ASSERT_EQ(map.at(p3), 52);
}

template<typename T>
static void WeakRef(T& map) {
    sm::shared_ref<Ints> p {sm::make_shared<Ints>(21, 30)};

    sm::shared_ref<int> p2 {p, &p->a};
    sm::shared_ref<int> p3 {p, &p->b};

    map[p2] = 52;
    map[sm::make_shared<int>(0)] = 0;

    ASSERT_EQ(map.at(p3), 52);
}

TEST(owner_less, SharedRef) {
    std::map<sm::shared_ref<int>, int, sm::owner_less<sm::shared_ref<int>>> map;

    SharedRef(map);
}

TEST(owner_less, WeakRef) {
    std::map<sm::shared_ref<int>, int, sm::owner_less<sm::shared_ref<int>>> map;

    WeakRef(map);
}

TEST(owner_less, Deduced) {
    {
        std::map<sm::shared_ref<int>, int, sm::owner_less<>> map;

        SharedRef(map);
    }

    {
        std::map<sm::weak_ref<int>, int, sm::owner_less<>> map;

        WeakRef(map);
    }
}
