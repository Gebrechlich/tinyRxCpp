#ifndef UTIL
#define UTIL
#include <memory>
#include <type_traits>

template<typename T>
struct is_iterable
{
    typedef char yes[1];
    typedef char no[2];

    template<typename C>
    static yes& test(typename C::const_iterator*);

    template<typename C>
    static no& test(...);

    static const bool value = (sizeof(test<T>(0))
                               == sizeof(yes));
};

template<typename T, typename... Ts>
std::unique_ptr<T> make_unique(Ts&&... params)
{
    return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}

template<typename T, typename = typename std::enable_if<std::is_array<T>::value
                                                        || is_iterable<T>::value>::type>
auto resolveConteinerValueType(const T& t) -> decltype(*std::begin(t))
{
    return *std::begin(t);
}

#endif // UTIL

