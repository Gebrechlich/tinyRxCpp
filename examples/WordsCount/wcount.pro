TEMPLATE = app
CONFIG += console
CONFIG -= qt
QMAKE_CXXFLAGS = -std=c++11
SOURCES += main.cpp

LIBS += -lgtest_main
LIBS += -lgtest
LIBS += -pthread

INCLUDEPATH += ../../src/operators
INCLUDEPATH += ../../src/schedulers
INCLUDEPATH += ../../src/utils
INCLUDEPATH += ../../src

HEADERS += \
    ../../src/operators/OperatorDistinct.hpp \
    ../../src/operators/OperatorFilter.hpp \
    ../../src/operators/OperatorMap.hpp \
    ../../src/Functions.hpp \
    ../../src/Observable.hpp \
    ../../src/Observer.hpp \
    ../../src/Subscriber.hpp \
    ../../src/operators/OperatorAll.hpp \
    ../../src/src/operators/OperatorExist.hpp \
    ../../src/operators/Operator.hpp \
    ../../src/operators/OperatorScan.hpp \
    ../../src/operators/OperatorLast.hpp \
    ../../src/Subscription.h \
    ../../src/Scheduler.hpp \
    ../../src/schedulers/NewThreadScheduler.hpp \
    ../../src/SchedulersFactory.hpp \
    ../../src/FunctionsPtr.hpp \
    ../../src/SubscriptionsList.hpp \
    ../../src/utils/Util.hpp \
    ../../src/operators/OperatorTake.hpp \
    ../../src/Producer.hpp \
    ../../src/operators/OperatorObserveOn.hpp \
    ../../src/utils/MTQueue.hpp \
    ../../src/utils/ThreadPoolExecutor.hpp \
    ../../src/operators/OperatorToMap.hpp
