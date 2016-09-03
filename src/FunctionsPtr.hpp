#ifndef FUNCTIONSPTR
#define FUNCTIONSPTR
#include "Functions.hpp"
#include "Util.hpp"
#include <memory>
#include <type_traits>
#include <iostream>

template<typename R>
struct Function0UniquePtr : public std::unique_ptr<Function0<R>>
{
    Function0UniquePtr(Function0<R> *ptr) :
        std::unique_ptr<Function0<R>>(ptr)
    {}

    Function0UniquePtr(std::unique_ptr<Function0<R>>&& o) :
        std::unique_ptr<Function0<R>>(std::move(o))
    {}

    template<typename U, typename = typename std::enable_if<
                 std::is_same<typename std::result_of<
                 U()>::type , R>::value &&
                 !std::is_same<std::nullptr_t, U>::value
                 >::type>
    Function0UniquePtr(U&& fp) :
        std::unique_ptr<Function0<R>>(make_unique<Function0<R>>(std::move(fp)))
    {}
};

template<typename R, typename T>
struct Function1UniquePtr : public std::unique_ptr<Function1<R,T>>
{
    Function1UniquePtr(Function1<R,T> *ptr) :
        std::unique_ptr<Function1<R,T>>(ptr)
    {}

    Function1UniquePtr(std::unique_ptr<Function1<R,T>>&& o) :
        std::unique_ptr<Function1<R,T>>(std::move(o))
    {}

    template<typename U, typename = typename std::enable_if<
                 std::is_same<typename std::result_of<U(const T&)>::type, R>::value &&
                 !std::is_same<std::nullptr_t, U>::value>::type>
    Function1UniquePtr(U&& fp) :
        std::unique_ptr<Function1<R,T>>(make_unique<Function1<R,T>>(std::move(fp)))
    {}
};

template<typename R, typename T0, typename T1>
struct Function2UniquePtr : public std::unique_ptr<Function2<R,T0,T1>>
{
    Function2UniquePtr(Function2<R,T0,T1> *ptr) :
        std::unique_ptr<Function2<R,T0,T1>>(ptr)
    {}

    Function2UniquePtr(std::unique_ptr<Function2<R,T0,T1>>&& o) :
        std::unique_ptr<Function2<R,T0,T1>>(std::move(o))
    {}

    template<typename U, typename = typename std::enable_if<
                       std::is_same<typename std::result_of<
                       U(const T0&, const T1&)>::type , R>::value &&
                       !std::is_same<std::nullptr_t, U>::value>::type>
    Function2UniquePtr(U&& fp) :
        std::unique_ptr<Function2<R,T0,T1>>(make_unique<Function2<R,T0,T1>>(std::move(fp)))
    {}
};

#endif // FUNCTIONSPTR

