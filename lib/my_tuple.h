#pragma once


#include <iostream>


template<typename... Args>
struct MyTuple;


template<typename Head, typename... Tail>
struct MyTuple<Head, Tail...> {
    Head head;
    MyTuple<Tail...> tail;

    MyTuple(Head &&h, Tail &&... t)
        : head(std::forward<Head>(h)), tail(std::forward<Tail>(t)...) {
    }
};


template<>
struct MyTuple<> {
};


template<size_t Index, typename Tuple>
struct MyGet;


template<size_t Index, typename Head, typename... Tail>
struct MyGet<Index, MyTuple<Head, Tail...> > {
    template<typename T>
    static decltype(auto) get(T &&t) {
        return MyGet<Index - 1, MyTuple<Tail...> >::get(std::forward<T>(t).tail);
    }
};


template<typename Head, typename... Tail>
struct MyGet<0, MyTuple<Head, Tail...> > {
    template<typename T>
    static decltype(auto) get(T &&t) {
        return std::forward<T>(t).head;
    }
};


template<size_t... Indices>
struct my_index_sequence {
};


template<size_t n, size_t... Indices>
struct my_make_index_sequence : my_make_index_sequence<n - 1, n - 1, Indices...> {
};


template<size_t... Indices>
struct my_make_index_sequence<0, Indices...> {
    using type = my_index_sequence<Indices...>;
};