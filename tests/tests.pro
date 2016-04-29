TEMPLATE = app
CONFIG += console
CONFIG -= qt
QMAKE_CXXFLAGS = -std=c++11
SOURCES += main.cpp

LIBS += -lpthread
LIBS += -lgtest_main
LIBS += -lgtest

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
    ../src/Subscription.h \
    ../src/Scheduler.hpp \
    ../src/schedulers/NewThreadScheduler.hpp \
    ../src/SchedulersFactory.hpp \
    ../src/FunctionsPtr.hpp \
    ../src/SubscriptionsList.hpp \
    ../src/utils/Util.hpp \
    ../src/operators/OperatorTake.hpp \
    ../src/utils/MTQueue.h \
    ../src/Producer.hpp
