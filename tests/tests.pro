TEMPLATE = app
CONFIG += console
CONFIG -= qt
QMAKE_CXXFLAGS = -std=c++11
SOURCES += main.cpp

LIBS += -lgtest_main
LIBS += -lgtest
LIBS += -pthread

INCLUDEPATH += ../src/operators
INCLUDEPATH += ../src/schedulers
INCLUDEPATH += ../src/utils
INCLUDEPATH += ../src

HEADERS += \
    ../src/operators/OperatorDistinct.hpp \
    ../src/operators/OperatorFilter.hpp \
    ../src/operators/OperatorMap.hpp \
    ../src/Functions.hpp \
    ../src/Observable.hpp \
    ../src/Observer.hpp \
    ../src/Subscriber.hpp \
    ../src/operators/OperatorAll.hpp \
    ../src/src/operators/OperatorExist.hpp \
    ../src/operators/Operator.hpp \
    ../src/operators/OperatorScan.hpp \
    ../src/operators/OperatorLast.hpp \
    ../src/Scheduler.hpp \
    ../src/schedulers/NewThreadScheduler.hpp \
    ../src/SchedulersFactory.hpp \
    ../src/utils/Util.hpp \
    ../src/operators/OperatorTake.hpp \
    ../src/operators/OperatorObserveOn.hpp \
    ../src/utils/MTQueue.hpp \
    ../src/utils/ThreadPoolExecutor.hpp \
    ../src/operators/OperatorToMap.hpp \
    ../src/operators/OperatorDoOnEach.hpp \
    ../src/operators/OperatorExist.hpp \
    ../src/operators/OnSubscribeBase.hpp \
    ../src/operators/LiftOnSubscribe.hpp \
    ../src/operators/OperatorSubscribeOn.hpp \
    ../src/operators/DeferOnSubscribe.hpp \
    ../src/operators/RangeOnSubscribe.hpp \
    ../src/operators/RepeatOnSubscribe.hpp \
    ../src/operators/OnSubscribeConcatMap.hpp \
    ../src/operators/OnSubscribePeriodically.hpp \
    ../src/TinyRxCpp.h \
    ../src/operators/OnSubscribeFlatMap.hpp \
    ../src/schedulers/ThreadPoolScheduler.hpp \
    ../src/Subscription.hpp \
    ../src/operators/OperatorTakeWhile.hpp \
    ../src/operators/OperatorSynchronize.hpp \
    ../src/exceptions/TRExceptions.hpp
