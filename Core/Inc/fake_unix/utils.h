#pragma once
#include <stddef.h>
#include <stdint.h>
namespace fake_unix
{
    template<class T,size_t n>
    constexpr size_t get_array_size(T(&)[n])
    {
        return n;
    }

    template<class T>
    class array_size;

    template<class T,size_t n>
    class array_size<T[n]>
    {
    public:
        static constexpr size_t value=n;
    };

    template<class T>
    class this2type;

    template<class T>
    class this2type<T*>
    {
    public:
        using type=T;
    };

} // namespace fake_unix
