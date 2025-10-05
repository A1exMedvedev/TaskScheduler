#include "scheduler.h"


#include <gmock/gmock.h>
#include <gtest/gtest.h>


struct ComplexFunctor {
    int operator()(int a, int b) {
        return a * b + 1;
    }
};


int global_sum(int a, int b) {
    return a + b;
}

float square(float x) {
    return x * x;
}


TEST(TaskSchedulerTest, CallableFunction) {
    TTaskScheduler scheduler;
    auto func = [](int x) { return x + 1; };
    auto id = scheduler.add(func, 10);
    EXPECT_EQ(scheduler.getResult<int>(id), 11);
}


TEST(TTaskSchedulerTest, ComplexDependencyChain) {
    TTaskScheduler scheduler;

    auto id1 = scheduler.add([] { return 3; });
    auto id2 = scheduler.add([] { return 4; });

    auto id3 = scheduler.add(global_sum, scheduler.getFutureResult<int>(id1), scheduler.getFutureResult<int>(id2));
    auto id4 = scheduler.add(square, scheduler.getFutureResult<int>(id3));

    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id1), 3);
    EXPECT_EQ(scheduler.getResult<int>(id2), 4);
    EXPECT_EQ(scheduler.getResult<int>(id3), 7);
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id4), 49.0f);
}


TEST(TTaskSchedulerTest, FunctorAndLambdaChaining) {
    TTaskScheduler scheduler;

    auto id1 = scheduler.add(ComplexFunctor(), 2, 5);
    auto id2 = scheduler.add([](int x) { return x * 2; }, scheduler.getFutureResult<int>(id1));
    auto id3 = scheduler.add([](int x, int y) { return x - y; },
                             scheduler.getFutureResult<int>(id2),
                             scheduler.getFutureResult<int>(id1));

    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id1), 11);
    EXPECT_EQ(scheduler.getResult<int>(id2), 22);
    EXPECT_EQ(scheduler.getResult<int>(id3), 11);
}



TEST(TTaskSchedulerTest, DeepChainedTasks) {
    TTaskScheduler scheduler;

    std::vector<TypeId> ids;
    ids.push_back(scheduler.add([] { return 1; }));

    for (int i = 1; i < 10; ++i) {
        ids.push_back(scheduler.add([](int x) { return x + 1; }, scheduler.getFutureResult<int>(ids.back())));
    }

    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(ids.front()), 1);
    EXPECT_EQ(scheduler.getResult<int>(ids.back()), 10);
}


int multiply(int a, int b) {
    return a * b;
}

TEST(TTaskSchedulerTest, FunctionPointerTest) {
    TTaskScheduler scheduler;

    int (*func_ptr)(int, int) = multiply;

    auto id = scheduler.add(func_ptr, 6, 7);

    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id), 42);
}



int subtract(int a, int b) {
    return a - b;
}

TEST(TTaskSchedulerTest, FunctionReferenceTest) {
    TTaskScheduler scheduler;

    int (&func_ref)(int, int) = subtract;

    auto id = scheduler.add(func_ref, 10, 4);

    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id), 6);
}


int sum(int a, int b) {
    return a + b;
}

TEST(TTaskSchedulerTest, FunctionPointerWithMultipleDependencies) {
    TTaskScheduler scheduler;

    auto id1 = scheduler.add(*subtract, 5, 2);
    auto id2 = scheduler.add([]() { return 7; });
    auto id3 = scheduler.add(&sum, scheduler.getFutureResult<int>(id1), scheduler.getFutureResult<int>(id2));

    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id3), 10);
}


int max(int a, int b) {
    return (a > b) ? a : b;
}

TEST(TTaskSchedulerTest, MixedFunctionPointerAndReference) {
    TTaskScheduler scheduler;

    int (*ptr)(int, int) = max;
    int (&ref)(int, int) = max;

    auto id1 = scheduler.add(ptr, 13, 42);
    auto id2 = scheduler.add(ref, 5, 2);

    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id1), 42);
    EXPECT_EQ(scheduler.getResult<int>(id2), 5);
}



TEST(TTaskSchedulerTest, MultiStageExecution) {
    TTaskScheduler scheduler;

    auto id1 = scheduler.add([]() { return 8; });
    auto id2 = scheduler.add([](int x) { return x + 2; }, scheduler.getFutureResult<int>(id1));
    auto id3 = scheduler.add([](int x) { return x * x; }, scheduler.getFutureResult<int>(id2));

    EXPECT_EQ(scheduler.getResult<int>(id3), 100);
}  


const std::string global_str = "initial";


const std::string& get_global_str() {
    return global_str;
}


TEST(TaskSchedulerTest, ReturnGlobalStringRef) {
    TTaskScheduler scheduler;

    auto id1 = scheduler.add(get_global_str);

    scheduler.executeAll();

    const std::string& result = scheduler.getResult<const std::string&>(id1);
    EXPECT_EQ(result, global_str);
}