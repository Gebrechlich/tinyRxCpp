#ifndef OPERATOR_H
#define OPERATOR_H
#include "../Functions.hpp"
#include "../Subscriber.hpp"
#include <memory>

template<typename R, typename P>
using Operator = Function1<std::shared_ptr<Subscriber<R>>, std::shared_ptr<Subscriber<P>>>;

#endif // OPERATOR_H
