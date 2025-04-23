#include "scheduler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>


TEST(TTaskSchedulerTest, LambdaChainingWithMultipleArguments) {
    TTaskScheduler scheduler;

    auto id1 = scheduler.add([](int a, int b) { return a + b; }, 3, 4);
    auto id2 = scheduler.add([](int x) { return x * x; }, scheduler.getFutureResult<int>(id1));
    auto id3 = scheduler.add([](int x, int y) { return x - y; },
                             scheduler.getFutureResult<int>(id1),
                             scheduler.getFutureResult<int>(id2));

    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id1), 7);
    EXPECT_EQ(scheduler.getResult<int>(id2), 49);
    EXPECT_EQ(scheduler.getResult<int>(id3), -42);
}


TEST(TTaskSchedulerTest, LambdaChangingState) {
    TTaskScheduler scheduler;

    auto id1 = scheduler.add([](int a) { return a + 1; }, 0);
    auto id2 = scheduler.add([](int x) { return x + 5; }, scheduler.getFutureResult<int>(id1));
    auto id3 = scheduler.add([](int x) { return x - 3; }, scheduler.getFutureResult<int>(id2));

    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id1), 1);
    EXPECT_EQ(scheduler.getResult<int>(id2), 6);
    EXPECT_EQ(scheduler.getResult<int>(id3), 3);
}


TEST(TTaskSchedulerTest, LambdaPointer) {
    TTaskScheduler scheduler;

    auto lambda = [](int a, int b) { return a + b; };
    auto* lambdaPtr = &lambda;

    auto id = scheduler.add(*lambdaPtr, 2, 3);
    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id), 5);
}



TEST(TTaskSchedulerTest, LambdaReference) {
    TTaskScheduler scheduler;

    auto lambda = [](int a, int b) { return a * b; };
    auto& lambdaRef = lambda;

    auto id = scheduler.add(lambdaRef, 4, 5);
    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id), 20);
}



TEST(TTaskSchedulerTest, LambdaTemporary) {
    TTaskScheduler scheduler;

    auto id = scheduler.add([](std::string s) { return s + "!"; }, std::string("Hello"));
    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<std::string>(id), "Hello!");
}


TEST(TTaskSchedulerTest, LambdaTakesPointer) {
    TTaskScheduler scheduler;

    auto id = scheduler.add([](int* ptr) {
        return *ptr + 5;
    }, new int(7));
    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id), 12);
}


TEST(TTaskSchedulerTest, LambdaReturningReference) {
    TTaskScheduler scheduler;

    std::vector<int> vec = {1, 2, 3};

    auto id = scheduler.add([&vec]() -> std::reference_wrapper<int> { return vec[1]; });
    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<std::reference_wrapper<int>>(id).get(), 2);

    scheduler.getResult<std::reference_wrapper<int>>(id).get() = 99;
    EXPECT_EQ(vec[1], 99);
}


