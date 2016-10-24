TEMPLATE = app
CONFIG += console
CONFIG -= qt
QMAKE_CXXFLAGS = -std=c++11
SOURCES += main.cpp

LIBS += -lgtest_main
LIBS += -lgtest
LIBS += -pthread

INCLUDEPATH += ../../src
