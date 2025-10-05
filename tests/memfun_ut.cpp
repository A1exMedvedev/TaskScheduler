#include "my_referense_wrapper.h"
#include "scheduler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>



struct Counter {
    int x = 3;
    int y = 7;

    int Sum() const {
        return x + y;
    }

    void IncX(int delta) {
        x += delta;
    }

    int& RefX() { return x; }

    MyReferenceWrapper<const int> ConstRefY() const { return MyRef(y); }

    int Mul(int factor) const {
        return (x + y) * factor;
    }

    int FieldAccess() const {
        return x;
    }
};



TEST(TTaskSchedulerTest, CallMethodViaPointer) {
    TTaskScheduler scheduler;
    Counter c;

    TypeId id = scheduler.add(&Counter::Sum, &c);

    int result = scheduler.getResult<int>(id);
    ASSERT_EQ(result, 10);
}


TEST(TTaskSchedulerTest, CallMethodWithArgsOnRef) {
    TTaskScheduler scheduler;
    Counter c;

    TypeId id = scheduler.add(&Counter::Mul, &c, 2);

    int result = scheduler.getResult<int>(id);
    ASSERT_EQ(result, 20);
}


TEST(TTaskSchedulerTest, NestedTaskUsingFutureResult) {
    TTaskScheduler scheduler;
    Counter c;

    TypeId id1 = scheduler.add(&Counter::Sum, &c);

    TypeId id2 = scheduler.add([](int sum) {
        return sum * 10;
    }, scheduler.getFutureResult<int>(id1));

    ASSERT_EQ(scheduler.getResult<int>(id2), 100);
}



TEST(TTaskSchedulerTest, LambdaCallingMemberFunction) {
    TTaskScheduler scheduler;
    Counter c;

    TypeId id = scheduler.add([&c]() {
        return c.Mul(3);
    });

    ASSERT_EQ(scheduler.getResult<int>(id), 30);
}


TEST(TTaskSchedulerTest, ModifyFieldThroughRefWrapper) {
    TTaskScheduler scheduler;
    Counter c;

    TypeId id1 = scheduler.add([](Counter* ptr) {
        return MyRef(ptr->RefX());
    }, &c);

    scheduler.getResult<MyReferenceWrapper<int>>(id1).get() = 42;

    TypeId id2 = scheduler.add(&Counter::FieldAccess, &c);
    ASSERT_EQ(scheduler.getResult<int>(id2), 42);
}


TEST(TTaskSchedulerTest, ConstRefToField) {
    TTaskScheduler scheduler;
    Counter c;

    TypeId id = scheduler.add(&Counter::ConstRefY, &c);

    const int& y = scheduler.getResult<MyReferenceWrapper<const int>>(id).get();
    ASSERT_EQ(y, 7);
}


class MyData {
public:
    int value = 10;
};
    


TEST(TTaskSchedulerTest, TaskWithPointerAndReference) {
    TTaskScheduler scheduler;

    const MyData data;
    const int &ref = data.value;
    const int *ptr = &data.value;

    auto id1 = scheduler.add([](const int &r) { return r + 1; }, ref);

    auto id2 = scheduler.add([](const int *p) { return *p * 2; }, ptr);

    scheduler.executeAll();

    EXPECT_EQ(scheduler.getResult<int>(id1), 11);
    EXPECT_EQ(scheduler.getResult<int>(id2), 20);
}


TEST(MyInvokeTest, TestConstFieldAccess) {
    TTaskScheduler scheduler;

    const MyData obj;
    
    auto id = scheduler.add(&MyData::value, obj);

    EXPECT_EQ(scheduler.getResult<int>(id), 10);
}




