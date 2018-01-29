QT += network

QMAKE_CXXFLAGS += -Werror -std=c++11
QMAKE_LFLAGS += -std=c++11

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/coap.h \
    $$PWD/coappdu.h \
    $$PWD/coapoption.h \
    $$PWD/coaprequest.h \
    $$PWD/coapreply.h \
    $$PWD/coappdublock.h \
    $$PWD/corelinkparser.h \
    $$PWD/corelink.h \
    $$PWD/coapobserveresource.h

SOURCES += \
    $$PWD/coap.cpp \
    $$PWD/coappdu.cpp \
    $$PWD/coapoption.cpp \
    $$PWD/coaprequest.cpp \
    $$PWD/coapreply.cpp \
    $$PWD/coappdublock.cpp \
    $$PWD/corelinkparser.cpp \
    $$PWD/corelink.cpp \
    $$PWD/coapobserveresource.cpp

