#include "scheduler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>



TEST(TTaskSchedulerTest, BadCastWrongTypee) {
    TTaskScheduler scheduler;

    auto id = scheduler.add([](float a, float b) {return a + b;}, 4, 5);

    scheduler.executeAll();

    EXPECT_THROW({ scheduler.getResult<double>(id); }, std::bad_cast);
}


TEST(TTaskSchedulerTest, CycleWrongTypee) {
    TTaskScheduler scheduler;

    size_t i = 1;

    auto id1 = *reinterpret_cast<TypeId *>(&i);

    EXPECT_THROW(
        {auto id = scheduler.add([](int a, int b) { return a + b; }, 4, scheduler.getFutureResult<int>(id1)); },
        MyCycleException);
}



TEST(TTaskSchedulerTest, ComplexCycleDetection) {
    TTaskScheduler scheduler;

    auto base = scheduler.add([]() { return 42; });

    auto next = scheduler.add([](int x) { return x * 2; },
            scheduler.getFutureResult<int>(base));

    auto third = scheduler.add([](int x, int y) { return x + y; },
        scheduler.getFutureResult<int>(base),
        scheduler.getFutureResult<int>(next));

    size_t fake = 1000;
    auto bad_id = *reinterpret_cast<TypeId *>(&fake);

    EXPECT_THROW({
        scheduler.add([](int a, int b) { return a * b; }, 10,
            scheduler.getFutureResult<int>(bad_id));
    }, MyCycleException);
}
