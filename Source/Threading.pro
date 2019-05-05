TEMPLATE = app
CONFIG += c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    Threading/Src/ThreadPool/DynamicCountThreadPool.cpp \
    Threading/Src/ThreadPool/ThreadPoolFactory.cpp \
    Threading/Src/Time/BusyTimer.cpp \
    Threading/Src/Time/BusyTimerImpl.cpp \
        main.cpp \
    Threading/Src/ThreadPool/PriorityThreadPool.cpp \
    Threading/Src/ThreadPool/StableThreadPool.cpp \
    Threading/Src/Time/Timer.cpp \
    Threading/Src/Time/Waiter.cpp \
    Threading/Src/Utils/Signal.cpp

INCLUDEPATH += ./Threading/Headers/

HEADERS += \
    Threading/Headers/Interfaces/BusyTimer.h \
    Threading/Headers/Interfaces/IThreadPool.h \
    Threading/Headers/Interfaces/Queue.h \
    Threading/Headers/Interfaces/Runnable.h \
    Threading/Headers/Interfaces/Signal.h \
    Threading/Headers/Interfaces/ThreadJoiner.h \
    Threading/Headers/Interfaces/ThreadPoolFactory.h \
    Threading/Headers/Interfaces/ThreadSafeQueue.h \
    Threading/Headers/Interfaces/Timer.h \
    Threading/Headers/Interfaces/Waiter.h \
    Threading/Headers/Prv/TP/DynamicCountThreadPool.h \
    Threading/Headers/Prv/TP/PriorityThreadPool.h \
    Threading/Headers/Prv/TP/StableThreadPool.h \
    Threading/Headers/Prv/TP/ThreadPoolImplBase.h \
    Threading/Headers/Prv/Time/BusyTimerImpl.h

LIBS += -lpthread

DISTFILES +=
