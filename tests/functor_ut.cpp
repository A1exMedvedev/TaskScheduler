#include "scheduler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>


struct Multiplier {
    int factor;
    int operator()(int x) const {
        return x * factor;
    }
};

TEST(TTaskSchedulerTest, FunctorObjectTest) {
    TTaskScheduler scheduler;

    Multiplier m{5};
    auto id = scheduler.add(m, 4);

    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id), 20);
}



TEST(TTaskSchedulerTest, FunctorReferenceTest) {
    TTaskScheduler scheduler;

    Multiplier m{7};
    Multiplier& ref = m;

    auto id = scheduler.add(ref, 3);

    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id), 21);
}



TEST(TTaskSchedulerTest, FunctorPointerTest) {
    TTaskScheduler scheduler;

    Multiplier m{10};
    Multiplier* ptr = &m;

    auto id = scheduler.add(*ptr, 6);

    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id), 60);
}



TEST(TTaskSchedulerTest, FunctorCompositionTest) {
    TTaskScheduler scheduler;

    struct Adder {
        int delta;
        int operator()(int x) const { return x + delta; }
    };

    Adder a{3};
    Multiplier m{2};

    auto id1 = scheduler.add(m, 4);
    auto id2 = scheduler.add(a, scheduler.getFutureResult<int>(id1));

    scheduler.executeAll();
    EXPECT_EQ(scheduler.getResult<int>(id2), 11);
}
