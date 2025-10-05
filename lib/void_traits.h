#pragma once


template<typename T>
struct is_void_type {
    static constexpr bool value = false;
};

template<>
struct is_void_type<void> {
    static constexpr bool value = true;
};


template<>
struct is_void_type<const void> {
    static constexpr bool value = true;
};


template<>
struct is_void_type<volatile void> {
    static constexpr bool value = true;
};


template<>
struct is_void_type<const volatile void> {
    static constexpr bool value = true;
};