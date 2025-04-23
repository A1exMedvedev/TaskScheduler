#include "scheduler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>


struct Math {
    int factor = 2;

    int multiply(int x) const { return x * factor; }
    int add(int x, int y) const { return x + y; }
};

TEST(TTaskSchedulerTest, BindWithArgument) {
    TTaskScheduler scheduler;
    Math m;

    auto id = scheduler.add(std::bind(&Math::multiply, &m, 10));
    int result = scheduler.getResult<int>(id);

    ASSERT_EQ(result, 20);
}


TEST(TTaskSchedulerTest, MemFnUsage) {
    TTaskScheduler scheduler;
    Math m{5};

    auto id = scheduler.add(std::mem_fn(&Math::multiply), &m, 3);
    int result = scheduler.getResult<int>(id);

    ASSERT_EQ(result, 15);
}


TEST(TTaskSchedulerTest, StdFunctionWrapsBind) {
    TTaskScheduler scheduler;
    Math m;

    std::function<int()> fn = std::bind(&Math::multiply, &m, 6);
    auto id = scheduler.add(fn);
    int result = scheduler.getResult<int>(id);

    ASSERT_EQ(result, 12);
}


TEST(TTaskSchedulerTest, StdFunctionWithCaptureLambda) {
    TTaskScheduler scheduler;
    int x = 10;

    std::function<int()> fn = [x]() { return x * 3; };
    auto id = scheduler.add(fn);
    int result = scheduler.getResult<int>(id);

    ASSERT_EQ(result, 30);
}


TEST(TTaskSchedulerTest, DependentBindChain) {
    TTaskScheduler scheduler;
    Math m;

    auto id1 = scheduler.add([] { return 4; });
    auto id2 = scheduler.add([] { return 5; });

    auto id3 = scheduler.add(std::bind(&Math::add, &m,
        scheduler.getFutureResult<int>(id1), scheduler.getFutureResult<int>(id2)));

    int result = scheduler.getResult<int>(id3);
    ASSERT_EQ(result, 9);
}


TEST(TTaskSchedulerTest, MixedMemFnAndLambda) {
    TTaskScheduler scheduler;
    Math m{3};

    auto id1 = scheduler.add([] { return 7; });
    auto id2 = scheduler.add(std::mem_fn(&Math::multiply), &m, scheduler.getFutureResult<int>(id1));

    int result = scheduler.getResult<int>(id2);
    ASSERT_EQ(result, 21);
}