#pragma once

#include "my_tuple.h"
#include "my_referense_wrapper.h"


template<typename Callable, typename... Args>
auto my_invoke(Callable &&f, Args &&... args)
    -> decltype(std::forward<Callable>(f)(std::forward<Args>(args)...)) {
    return std::forward<Callable>(f)(std::forward<Args>(args)...);
}


template<typename MemFun, typename Class, typename... Args>
auto my_invoke(MemFun &&f, Class &&obj, Args &&... args)
    -> decltype((std::forward<Class>(obj).*std::forward<MemFun>(f))(std::forward<Args>(args)...)) {
    return (std::forward<Class>(obj).*std::forward<MemFun>(f))(std::forward<Args>(args)...);
}


template<typename MemFun, typename Class, typename... Args>
auto my_invoke(MemFun &&f, Class &&obj, Args &&... args)
    -> decltype((*(std::forward<Class>(obj)).*std::forward<MemFun>(f))(std::forward<Args>(args)...)) {
    return (*(std::forward<Class>(obj)).*std::forward<MemFun>(f))(std::forward<Args>(args)...);
}


template<typename MemPtr, typename Class>
auto my_invoke(MemPtr &&f, Class &&obj)
    -> decltype(std::forward<Class>(obj).*std::forward<MemPtr>(f)) {
    return std::forward<Class>(obj).*std::forward<MemPtr>(f);
}


template<typename MemPtr, typename Class>
auto my_invoke(MemPtr &&f, Class &&obj)
    -> decltype((*std::forward<Class>(obj)).*std::forward<MemPtr>(f)) {
    return (*std::forward<Class>(obj)).*std::forward<MemPtr>(f);
}


template<typename MemFun, typename T, typename... Args>
auto my_invoke(MemFun &&f, MyReferenceWrapper<T> obj, Args &&... args)
    -> decltype((obj.get().*f)(std::forward<Args>(args)...)) {
    return (obj.get().*f)(std::forward<Args>(args)...);
}


template<typename MemPtr, typename T>
auto my_invoke(MemPtr &&f, MyReferenceWrapper<T> obj)
    -> decltype(obj.get().*f) {
    return obj.get().*f;
}



template<typename Func, typename Tuple, size_t... Indices>
auto my_tuple_apply_impl(Func &&f, Tuple &&tuple, my_index_sequence<Indices...>) {
    return my_invoke(std::forward<Func>(f), MyGet<Indices, Tuple>::get(std::forward<Tuple>(tuple))...);
}



template<typename Func, typename... Args>
auto my_tuple_apply(Func &&f, MyTuple<Args...> &&tuple) {
    using Indices = typename my_make_index_sequence<sizeof...(Args)>::type;
    return my_tuple_apply_impl(std::forward<Func>(f), std::forward<MyTuple<Args...> >(tuple), Indices{});
}