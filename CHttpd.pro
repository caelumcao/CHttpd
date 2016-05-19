TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    server.cpp \
    config.cpp \
    socketqueue.cpp \
    requestprotocol.cpp \
    utilities.cpp

LIBS += -lpthread \
        -lunp

HEADERS += \
    server.h \
    config.h \
    socketqueue.h \
    requestprotocol.h \
    utilities.h
