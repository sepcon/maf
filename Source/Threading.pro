TEMPLATE = app
CONFIG += c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    src/Application/Framework/Component.cpp \
    src/Threading/ThreadPool/DynamicCountThreadPool.cpp \
    src/Threading/ThreadPool/ThreadPoolFactory.cpp \
    src/Threading/Time/BusyTimer.cpp \
    src/Threading/Time/BusyTimerImpl.cpp \
        main.cpp \
    src/Threading/ThreadPool/PriorityThreadPool.cpp \
    src/Threading/ThreadPool/StableThreadPool.cpp \
    src/Threading/Time/Timer.cpp \
    src/Threading/Time/Waiter.cpp \
    src/Threading/Utils/Signal.cpp

INCLUDEPATH += ./headers/Threading/

HEADERS += \
    headers/Application/Framework/Component.h \
    headers/Threading/Interfaces/BusyTimer.h \
    headers/Threading/Interfaces/IThreadPool.h \
    headers/Threading/Interfaces/Queue.h \
    headers/Threading/Interfaces/Runnable.h \
    headers/Threading/Interfaces/Signal.h \
    headers/Threading/Interfaces/ThreadJoiner.h \
    headers/Threading/Interfaces/ThreadPoolFactory.h \
    headers/Threading/Interfaces/ThreadSafeQueue.h \
    headers/Threading/Interfaces/Timer.h \
    headers/Threading/Interfaces/Waiter.h \
    headers/Threading/Prv/TP/DynamicCountThreadPool.h \
    headers/Threading/Prv/TP/PriorityThreadPool.h \
    headers/Threading/Prv/TP/StableThreadPool.h \
    headers/Threading/Prv/TP/ThreadPoolImplBase.h \
    headers/Threading/Prv/Time/BusyTimerImpl.h

LIBS += -lpthread

DISTFILES +=
