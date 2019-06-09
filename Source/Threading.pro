TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++17 console

QMAKE_CXXFLAGS += -std=c++17 -O0
SOURCES += \
    src/Application/Framework/AppComponent.cpp \
    src/Application/Framework/Component.cpp \
    src/Application/Framework/Timer.cpp \
    src/Threading/ThreadPool/DynamicCountThreadPool.cpp \
    src/Threading/ThreadPool/ThreadPoolFactory.cpp \
    src/Threading/Time/BusyTimer.cpp \
    src/Threading/Time/BusyTimerImpl.cpp \
        main.cpp \
    src/Threading/ThreadPool/PriorityThreadPool.cpp \
    src/Threading/ThreadPool/StableThreadPool.cpp \
    src/Threading/Time/Waiter.cpp \
    src/Threading/Utils/Signal.cpp \
    src/Utils/IDManager.cpp

INCLUDEPATH += ./headers/Threading/

HEADERS += \
    headers/Application/Framework/AppComponent.h \
    headers/Application/Framework/Component.h \
    headers/Application/Framework/MessageHandler.h \
    headers/Application/Framework/Messages.h \
    headers/Application/Framework/Timer.h \
    headers/Messaging/Message.h \
    headers/Threading/Interfaces/BusyTimer.h \
    headers/Threading/Interfaces/IThreadPool.h \
    headers/Threading/Interfaces/Queue.h \
    headers/Threading/Interfaces/Runnable.h \
    headers/Threading/Interfaces/Signal.h \
    headers/Threading/Interfaces/ThreadJoiner.h \
    headers/Threading/Interfaces/ThreadPoolFactory.h \
    headers/Threading/Interfaces/ThreadSafeQueue.h \
    headers/Threading/Interfaces/Waiter.h \
    headers/Threading/Prv/TP/DynamicCountThreadPool.h \
    headers/Threading/Prv/TP/PriorityThreadPool.h \
    headers/Threading/Prv/TP/StableThreadPool.h \
    headers/Threading/Prv/TP/ThreadPoolImplBase.h \
    headers/Threading/Prv/Time/BusyTimerImpl.h \
    headers/Utils/IDManager.h

LIBS += -lpthread

DISTFILES +=
