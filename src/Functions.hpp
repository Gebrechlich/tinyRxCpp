#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <functional>
#include "utils/Util.hpp"
#include <type_traits>
#include <memory>

using Action0_t = std::function<void(void)>;

template<typename T>
using Action1_t = std::function<void(const T&)>;

template<typename R>
using Function0_t = std::function<R(void)>;

template<typename R, typename T>
using Function1_t = std::function<R(const T&)>;

template<typename R, typename T0, typename T1>
using Function2_t = std::function<R(const T0&, const T1&)>;

struct Function
{
    virtual ~Function(){}
};

struct Action0 : public Function
{
    using ActionFp = Action0_t;
    ActionFp actionFp;

    Action0() = default;
    Action0(Action0&&) = default;
    Action0& operator = (Action0&&) = default;
    Action0(ActionFp fp) : actionFp(fp){}
    virtual ~Action0() = default;

    virtual void operator()()
    {
        if(actionFp)
        {
            actionFp();
        }
    }
};

using ActionRefType = std::shared_ptr<Action0>;

template<typename T>
struct Action1 : public Function
{
    using ActionFp = Action1_t<T>;
    ActionFp actionFp;

    Action1() = default;
    Action1(Action1&&) = default;
    Action1& operator = (Action1&&) = default;
    Action1(ActionFp fp) : actionFp(fp){}
    virtual ~Action1() = default;

    virtual void operator()(const T& t)
    {
        if(actionFp)
        {
            actionFp(t);
        }
    }
};

template<typename T>
using Action1RefType = std::shared_ptr<Action1<T>>;

template<typename R, typename T>
struct Function1 : public Function
{
    using FunctionFp = Function1_t<R, T>;
    FunctionFp actionFp;

    Function1() = default;
    Function1(Function1&&) = default;
    Function1& operator = (Function1&&) = default;
    Function1(FunctionFp fp) : actionFp(fp){}
    virtual ~Function1() = default;

    virtual R operator()(const T& t)
    {
        if(actionFp)
        {
            return actionFp(t);
        }
        return R();
    }
};

template<typename R, typename T0, typename T1>
struct Function2 : public Function
{
    using FunctionFp = Function2_t<R,T0,T1>;
    FunctionFp actionFp;

    Function2() = default;
    Function2(Function2&&) = default;
    Function2& operator = (Function2&&) = default;
    Function2(FunctionFp fp) : actionFp(fp){}
    virtual ~Function2() = default;

    virtual R operator()(const T0& t0, const T1& t1)
    {
        if(actionFp)
        {
            return actionFp(t0, t1);
        }
        return R();
    }
};

#endif // FUNCTIONS_H
