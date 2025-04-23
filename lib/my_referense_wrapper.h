#pragma once

#include <iostream>


template<typename T>
class MyReferenceWrapper {
public:
    using type = T;

    MyReferenceWrapper(T &ref) : ptr_(&ref) {
    }

    MyReferenceWrapper(const MyReferenceWrapper &other) = default;

    MyReferenceWrapper& operator=(const MyReferenceWrapper &other) = default;

    operator T &() const {
        return *ptr_;
    }

    T &get() const {
        return *ptr_;
    }

    template<typename... Args>
    decltype(auto) operator()(Args &&... args) const {
        return (*ptr_)(std::forward<Args>(args)...);
    }

private:
    T *ptr_;
};



template<typename T>
MyReferenceWrapper<T> MyRef(T &t) {
    return MyReferenceWrapper<T>(t);
}



template <typename T>
struct my_remove_reference {
    using type = T;
};

template <typename T>
struct my_remove_reference<T&> {
    using type = T;
};

template <typename T>
struct my_remove_reference<T&&> {
    using type = T;
};


template <typename T>
struct my_remove_reference<const T&> {
    using type = T;
};