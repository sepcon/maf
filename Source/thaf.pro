
TARGET = thaf
TEMPLATE = lib
CONFIG += staticlib

QT       -= core gui

DEFINES += ENABLE_IPC_DEBUG=2

#DEFINES += MESSAGING_BY_PRIORITY

win32: {
    INCLUDEPATH += $(VC_IncludePath) $(WindowsSDK_IncludePath)
    message(win32)
    LIBS += -lKernel32

    SOURCES += src/thaf/Messaging/IPC/platforms/Windows/LocalIPCReceiver.cpp \
                src/thaf/Messaging/IPC/platforms/Windows/LocalIPCSender.cpp \
                src/thaf/Messaging/IPC/platforms/Windows/OverlappedPipeReceiver.cpp


    HEADERS +=    

    mingw {
        message(Mingw)
        QMAKE_CXXFLAGS += -std=c++17 -O0
        LIBS += -lpthread
    }
    msvc {
#        CONFIG += console
        QMAKE_CXXFLAGS += /std:c++17
    }

} else {
    message(Not win32)
    QMAKE_CXXFLAGS += -std=c++17 -O0
    LIBS += -lpthread
}

SOURCES += \
    src/thaf/Application/AppComponent.cpp \
    src/thaf/Application/Component.cpp \
    src/thaf/Application/IPCClient.cpp \
    src/thaf/Application/IPCMessages.cpp \
    src/thaf/Application/IPCServer.cpp \
    src/thaf/Application/Timer.cpp \
    src/thaf/Logging/ConsoleLogger.cpp \
    src/thaf/Logging/LoggingComponent.cpp \
    src/thaf/Messaging/IPC/Address.cpp \
    src/thaf/Messaging/IPC/IPCCommunicator.cpp \
    src/thaf/Messaging/IPC/IPCFactory.cpp \
    src/thaf/Messaging/IPC/IPCMessage.cpp \
    src/thaf/Messaging/IPC/IPCRequestTracker.cpp \
    src/thaf/Messaging/IPC/IPCServiceProxy.cpp \
    src/thaf/Messaging/IPC/IPCServiceStub.cpp \
    src/thaf/Messaging/IPC/platforms/Windows/NamedPipeReceiver.cpp \
    src/thaf/Messaging/IPC/platforms/Windows/NamedPipeReceiverBase.cpp \
    src/thaf/Messaging/IPC/platforms/Windows/NamedPipeSender.cpp \
    src/thaf/Messaging/IPC/platforms/Windows/NamedPipeSenderBase.cpp \
    src/thaf/Messaging/IPC/platforms/Windows/OverlappedPipeSender.cpp \
    src/thaf/Threading/Thread.cpp \
    src/thaf/Threading/ThreadPool/DynamicCountThreadPool.cpp \
    src/thaf/Threading/ThreadPool/ThreadPoolFactory.cpp \
    src/thaf/Threading/ThreadPool/PriorityThreadPool.cpp \
    src/thaf/Threading/ThreadPool/StableThreadPool.cpp \
    src/thaf/Threading/Time/TimerManager.cpp \
    src/thaf/Threading/Time/TimerManagerImpl.cpp \
    src/thaf/Threading/Time/Waiter.cpp \
    src/thaf/Utils/IDManager.cpp \
    src/thaf/Utils/Serialization/Serializer.cpp \
    src/thaf/Messaging/IPC/IPCReceiver.cpp

INCLUDEPATH += ./include

HEADERS += \
    include/thaf/Application/AppComponent.h \
    include/thaf/Application/Application.h \
    include/thaf/Application/BasicMessages.h \
    include/thaf/Application/Component.h \
    include/thaf/Application/IPCClient.h \
    include/thaf/Application/IPCMessages.h \
    include/thaf/Application/IPCServer.h \
    include/thaf/Application/Timer.h \
    include/thaf/Logging/ConsoleLogger.h \
    include/thaf/Logging/LoggerBase.h \
    include/thaf/Logging/LoggerInterface.h \
    include/thaf/Logging/LoggingComponent.h \
    include/thaf/Messaging/IPC/Address.h \
    include/thaf/Messaging/IPC/CSContractMC.h \
    include/thaf/Messaging/IPC/ClientServerContract.h \
    include/thaf/Messaging/IPC/ClientServerContractMacros.h \
    include/thaf/Messaging/IPC/Connection.h \
    include/thaf/Messaging/IPC/IPCCommunicator.h \
    include/thaf/Messaging/IPC/IPCFactory.h \
    include/thaf/Messaging/IPC/IPCInfo.h \
    include/thaf/Messaging/IPC/IPCMessage.h \
    include/thaf/Messaging/IPC/IPCReceiver.h \
    include/thaf/Messaging/IPC/IPCRequestTracker.h \
    include/thaf/Messaging/IPC/IPCSender.h \
    include/thaf/Messaging/IPC/IPCServiceProxy.h \
    include/thaf/Messaging/IPC/IPCServiceStub.h \
    include/thaf/Messaging/IPC/MessageValidator.h \
    include/thaf/Messaging/IPC/cscmbk.h \
    include/thaf/Messaging/IPC/cscmrs.h \
    include/thaf/Messaging/IPC/Prv/LocalIPCReceiver.h \
    include/thaf/Messaging/IPC/Prv/LocalIPCSender.h \
    include/thaf/Messaging/IPC/Prv/Platforms/Windows/LocalIPCReceiver.h \
    include/thaf/Messaging/IPC/Prv/Platforms/Windows/LocalIPCSender.h \
    include/thaf/Messaging/IPC/Prv/Platforms/Windows/NamedPipeReceiver.h \
    include/thaf/Messaging/IPC/Prv/Platforms/Windows/NamedPipeReceiverBase.h \
    include/thaf/Messaging/IPC/Prv/Platforms/Windows/NamedPipeSender.h \
    include/thaf/Messaging/IPC/Prv/Platforms/Windows/NamedPipeSenderBase.h \
    include/thaf/Messaging/IPC/Prv/Platforms/Windows/OverlappedPipeReceiver.h \
    include/thaf/Messaging/IPC/Prv/Platforms/Windows/OverlappedPipeSender.h \
    include/thaf/Messaging/IPC/Prv/Platforms/Windows/PipeShared.h \
    include/thaf/Messaging/Message.h \
    include/thaf/Messaging/MessageHandler.h \
    include/thaf/Threading/IThreadPool.h \
    include/thaf/Threading/Queue.h \
    include/thaf/Threading/Runnable.h \
    include/thaf/Threading/Signal.h \
    include/thaf/Threading/Thread.h \
    include/thaf/Threading/ThreadJoiner.h \
    include/thaf/Threading/ThreadPoolFactory.h \
    include/thaf/Threading/ThreadSafeQueue.h \
    include/thaf/Threading/TimerManager.h \
    include/thaf/Threading/Waiter.h \
    include/thaf/Threading/Prv/TP/DynamicCountThreadPool.h \
    include/thaf/Threading/Prv/TP/PriorityThreadPool.h \
    include/thaf/Threading/Prv/TP/StableThreadPool.h \
    include/thaf/Threading/Prv/TP/ThreadPoolImplBase.h \
    include/thaf/Threading/Prv/Time/TimerManagerImpl.h \
    include/thaf/Utils/CppExtension/AtomicContainer.h \
    include/thaf/Utils/Debugging/Debug.h \
    include/thaf/Utils/IDManager.h \
    include/thaf/Patterns/Patterns.h \
    include/thaf/Utils/CppExtension/Macros.h \
    include/thaf/Utils/CppExtension/TupleManip.h \
    include/thaf/Utils/CppExtension/TypeTraits.h \
    include/thaf/Utils/Serialization/ByteArray.h \
    include/thaf/Utils/Serialization/DumpHelper.h \
    include/thaf/Utils/Serialization/SerializableObjMacros.h \
    include/thaf/Utils/Serialization/SerializableObject.h \
    include/thaf/Utils/Serialization/Serialization.h \
    include/thaf/Utils/Serialization/Serializer.h


