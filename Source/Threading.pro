TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++17

win32-msvc* {
message(Compile with msvc)
    QMAKE_CXXFLAGS += /std=c++17 /O0
} else {
message(compile with gcc)
    QMAKE_CXXFLAGS += -std=c++17 -O0
}

SOURCES += \
    src/Framework/Application/AppComponent.cpp \
    src/Framework/Application/Component.cpp \
    src/Framework/Application/Timer.cpp \
    src/Framework/Logging/ConsoleLogger.cpp \
    src/Framework/Logging/LoggerBase.cpp \
    src/Framework/Logging/LoggingComponent.cpp \
    src/Libs/Threading/Thread.cpp \
    src/Libs/Threading/ThreadPool/DynamicCountThreadPool.cpp \
    src/Libs/Threading/ThreadPool/ThreadPoolFactory.cpp \
    src/Libs/Threading/Time/BusyTimer.cpp \
    src/Libs/Threading/Time/BusyTimerImpl.cpp \
    src/Libs/Threading/ThreadPool/PriorityThreadPool.cpp \
    src/Libs/Threading/ThreadPool/StableThreadPool.cpp \
    src/Libs/Threading/Time/Waiter.cpp \
    src/Libs/Utils/IDManager.cpp \
    main.cpp

INCLUDEPATH += ./headers/Libs/Threading/

HEADERS += \
    headers/Framework/Application/AppComponent.h \
    headers/Framework/Application/Application.h \
    headers/Framework/Application/Component.h \
    headers/Framework/Application/Messages.h \
    headers/Framework/Application/Timer.h \
    headers/Framework/Logging/ConsoleLogger.h \
    headers/Framework/Logging/LoggerBase.h \
    headers/Framework/Logging/LoggerInterface.h \
    headers/Framework/Logging/LoggingComponent.h \
    headers/Framework/Messaging/IPC/Interfaces/Address.h \
    headers/Framework/Messaging/IPC/Interfaces/IPCClient.h \
    headers/Framework/Messaging/IPC/Interfaces/IPCCommunicator.h \
    headers/Framework/Messaging/IPC/Interfaces/IPCReceiver.h \
    headers/Framework/Messaging/IPC/Interfaces/IPCSender.h \
    headers/Framework/Messaging/IPC/Interfaces/IPCService.h \
    headers/Framework/Messaging/IPC/Interfaces/MessageValidator.h \
    headers/Framework/Messaging/Message.h \
    headers/Framework/Messaging/MessageHandler.h \
    headers/Libs/Threading/Interfaces/BusyTimer.h \
    headers/Libs/Threading/Interfaces/IThreadPool.h \
    headers/Libs/Threading/Interfaces/Queue.h \
    headers/Libs/Threading/Interfaces/Runnable.h \
    headers/Libs/Threading/Interfaces/Signal.h \
    headers/Libs/Threading/Interfaces/Thread.h \
    headers/Libs/Threading/Interfaces/ThreadJoiner.h \
    headers/Libs/Threading/Interfaces/ThreadPoolFactory.h \
    headers/Libs/Threading/Interfaces/ThreadSafeQueue.h \
    headers/Libs/Threading/Interfaces/Waiter.h \
    headers/Libs/Threading/Prv/TP/DynamicCountThreadPool.h \
    headers/Libs/Threading/Prv/TP/PriorityThreadPool.h \
    headers/Libs/Threading/Prv/TP/StableThreadPool.h \
    headers/Libs/Threading/Prv/TP/ThreadPoolImplBase.h \
    headers/Libs/Threading/Prv/Time/BusyTimerImpl.h \
    headers/Libs/Utils/IDManager.h \
    headers/Libs/Patterns/Patterns.h \
    headers/Libs/Utils/CppExtension/Macros.h \
    headers/Libs/Utils/CppExtension/TupleManip.h \
    headers/Libs/Utils/CppExtension/TypeTraits.h \
    headers/Libs/Utils/Serialization/ByteArray.h \
    headers/Libs/Utils/Serialization/SerializableObjMacros.h \
    headers/Libs/Utils/Serialization/SerializableObject.h \
    headers/Libs/Utils/Serialization/Serialization.h \
    headers/Libs/Utils/Serialization/Serializer.h

LIBS += -lpthread

