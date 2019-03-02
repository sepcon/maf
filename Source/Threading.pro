TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
    Threading/Signal.cpp \
    Threading/StableThreadPool.cpp \
    Threading/Timer.cpp \
    Threading/VaryCountThreadPool.cpp \
    Threading/PriorityThreadPool.cpp \
    Threading/Waiter.cpp

HEADERS += \
    Threading/IThreadPool.h \
    Threading/Signal.h \
    Threading/StableThreadPool.h \
    Threading/TheadSafeQueue.h \
    Threading/ThreadJoiner.h \
    Threading/Timer.h \
    Threading/VaryCountThreadPool.h \
    Threading/Queue.h \
    Threading/PriorityThreadPool.h \
    Threading/Waiter.h

DISTFILES +=
