
#TARGET = thaf
#TEMPLATE = lib
#CONFIG += staticlib

#QT       -= core gui

TARGET = thaf
TEMPLATE = app

QT       -= core gui

CONFIG += c++17

DEFINES += ENABLE_IPC_DEBUG=2

#DEFINES += MESSAGING_BY_PRIORITY

win32: {
    INCLUDEPATH += $(VC_IncludePath) $(WindowsSDK_IncludePath)
    message(win32)
    LIBS += -lKernel32

    SOURCES += src/thaf/messaging/ipc/platforms/windows/LocalIPCReceiver.cpp \
                src/thaf/messaging/ipc/platforms/windows/LocalIPCSender.cpp \
                src/thaf/messaging/ipc/platforms/windows/NamedPipeReceiver.cpp \
                src/thaf/messaging/ipc/platforms/windows/NamedPipeReceiverBase.cpp \
                src/thaf/messaging/ipc/platforms/windows/NamedPipeSender.cpp \
                src/thaf/messaging/ipc/platforms/windows/NamedPipeSenderBase.cpp \
                src/thaf/messaging/ipc/platforms/windows/OverlappedPipeSender.cpp \
                src/thaf/messaging/ipc/platforms/windows/OverlappedPipeReceiver.cpp


    HEADERS +=    

    mingw {
        message(Mingw)
        QMAKE_CXXFLAGS += -std=c++17 -O0
        LIBS += -lpthread
    }
    msvc {
        CONFIG += console
        QMAKE_CXXFLAGS += /std:c++17
    }

} else {
    message(Not win32)
    QMAKE_CXXFLAGS += -std=c++17
    LIBS += -lpthread
}

#    C:/Users/sepcon/Documents/qtproject/untitled2/Untitled2.cpp \

INCLUDEPATH += ./include

SOURCES += \
    src/thaf/main.cpp \
    src/thaf/messaging/Component.cpp \
    src/thaf/messaging/Timer.cpp \
    src/thaf/logging/ConsoleLogger.cpp \
    src/thaf/logging/LoggingComponent.cpp \
    src/thaf/messaging/client-server/CSMessage.cpp \
    src/thaf/messaging/client-server/ClientBase.cpp \
    src/thaf/messaging/client-server/IAMessageRouter.cpp \
    src/thaf/messaging/client-server/IAServiceProxy.cpp \
    src/thaf/messaging/client-server/IAServiceStub.cpp \
    src/thaf/messaging/client-server/RequestKeeper.cpp \
    src/thaf/messaging/client-server/ServerBase.cpp \
    src/thaf/messaging/client-server/ServiceProxyBase.cpp \
    src/thaf/messaging/client-server/ServiceStubBase.cpp \
    src/thaf/messaging/client-server/Address.cpp \
    src/thaf/messaging/ipc/BytesCommunicator.cpp \
    src/thaf/messaging/ipc/IPCClientBase.cpp \
    src/thaf/messaging/ipc/IPCServerBase.cpp \
    src/thaf/threading/Thread.cpp \
    src/thaf/threading/threadpool/DynamicCountThreadPool.cpp \
    src/thaf/threading/threadpool/ThreadPoolFactory.cpp \
    src/thaf/threading/threadpool/PriorityThreadPool.cpp \
    src/thaf/threading/threadpool/StableThreadPool.cpp \
    src/thaf/threading/time/TimerManager.cpp \
    src/thaf/threading/time/TimerManagerImpl.cpp \
    src/thaf/threading/time/Waiter.cpp \
    src/thaf/utils/IDManager.cpp \
    src/thaf/utils/serialization/Serializer.cpp \
    src/thaf/messaging/ipc/IPCFactory.cpp \
    src/thaf/messaging/ipc/IPCMessage.cpp \
    src/thaf/messaging/ipc/IPCReceiver.cpp

HEADERS += \
    include/thaf/messaging/BasicMessages.h \
    include/thaf/messaging/Component.h \
    include/thaf/messaging/LocalIPCServer.h \
    include/thaf/messaging/MessageBase.h \
    include/thaf/messaging/MessageQueue.h \
    include/thaf/messaging/Timer.h \
    include/thaf/logging/ConsoleLogger.h \
    include/thaf/logging/LoggerBase.h \
    include/thaf/logging/LoggerInterface.h \
    include/thaf/logging/LoggingComponent.h \
    include/thaf/messaging/Communicator.h \
    include/thaf/messaging/client-server/ClientBase.h \
    include/thaf/messaging/client-server/ClientServerContract.h \
    include/thaf/messaging/client-server/IAMessageRouter.h \
    include/thaf/messaging/client-server/IAServiceProxy.h \
    include/thaf/messaging/client-server/IAServiceStub.h \
    include/thaf/messaging/client-server/QueueingServiceProxy.h \
    include/thaf/messaging/client-server/QueueingServiceStub.h \
    include/thaf/messaging/client-server/ServerBase.h \
    include/thaf/messaging/client-server/ServiceProxyBase.h \
    include/thaf/messaging/client-server/ServiceStubBase.h \
    include/thaf/messaging/client-server/interfaces/Address.h \
    include/thaf/messaging/client-server/interfaces/CSMessage.h \
    include/thaf/messaging/client-server/interfaces/CSMessageReceiver.h \
    include/thaf/messaging/client-server/interfaces/CSStatus.h \
    include/thaf/messaging/client-server/interfaces/CSTypes.h \
    include/thaf/messaging/client-server/interfaces/ClientInterface.h \
    include/thaf/messaging/client-server/interfaces/RegisterDataStructure.h \
    include/thaf/messaging/client-server/interfaces/RequestKeeper.h \
    include/thaf/messaging/client-server/interfaces/ServerInterface.h \
    include/thaf/messaging/client-server/interfaces/ServiceMessageReceiver.h \
    include/thaf/messaging/client-server/interfaces/ServiceProviderInterface.h \
    include/thaf/messaging/client-server/interfaces/ServiceProxyInterface.h \
    include/thaf/messaging/client-server/interfaces/ServiceRequesterInterface.h \
    include/thaf/messaging/client-server/interfaces/ServiceStatusObserverInterface.h \
    include/thaf/messaging/client-server/interfaces/ServiceStubHandlerInterface.h \
    include/thaf/messaging/client-server/interfaces/ServiceStubInterface.h \
    include/thaf/messaging/client-server/prv/CSMessageTrait.h \
    include/thaf/messaging/client-server/prv/IAMessageTrait.h \
    include/thaf/messaging/client-server/Connection.h \
    include/thaf/messaging/MessageHandler.h \
    include/thaf/messaging/client-server/prv/ServiceManagement.h \
    include/thaf/messaging/ipc/BytesCommunicator.h \
    include/thaf/messaging/ipc/IPCClientBase.h \
    include/thaf/messaging/ipc/IPCMsgDefinesMacros.h \
    include/thaf/messaging/ipc/IPCServerBase.h \
    include/thaf/messaging/ipc/IPCTypes.h \
    include/thaf/messaging/ipc/LocalIPCClient.h \
    include/thaf/messaging/ipc/LocalIPCServer.h \
    include/thaf/messaging/ipc/LocalIPCServiceProxy.h \
    include/thaf/messaging/ipc/LocalIPCServiceStub.h \
    include/thaf/messaging/ipc/prv/platforms/linux/LocalIPCSender.h \
    include/thaf/threading/IThreadPool.h \
    include/thaf/threading/Queue.h \
    include/thaf/threading/Runnable.h \
    include/thaf/threading/Signal.h \
    include/thaf/threading/Thread.h \
    include/thaf/threading/ThreadJoiner.h \
    include/thaf/threading/ThreadPoolFactory.h \
    include/thaf/threading/ThreadSafeQueue.h \
    include/thaf/threading/TimerManager.h \
    include/thaf/threading/Waiter.h \
    include/thaf/threading/prv/DynamicCountThreadPool.h \
    include/thaf/threading/prv/PriorityThreadPool.h \
    include/thaf/threading/prv/StableThreadPool.h \
    include/thaf/threading/prv/ThreadPoolImplBase.h \
    include/thaf/threading/prv/TimerManagerImpl.h \
    include/thaf/utils/TimeMeasurement.h \
    include/thaf/utils/cppextension/SyncObject.h \
    include/thaf/utils/debugging/Debug.h \
    include/thaf/utils/IDManager.h \
    include/thaf/patterns/Patterns.h \
    include/thaf/utils/cppextension/Macros.h \
    include/thaf/utils/cppextension/TupleManip.h \
    include/thaf/utils/cppextension/TypeTraits.h \
    include/thaf/utils/serialization/ByteArray.h \
    include/thaf/utils/serialization/DumpHelper.h \
    include/thaf/utils/serialization/SerializableObjMacros.h \
    include/thaf/utils/serialization/SerializableObject.h \
    include/thaf/utils/serialization/Serialization.h \
    include/thaf/utils/serialization/Serializer.h \
    include/thaf/messaging/ipc/ClientServerContractMacros.h \
    include/thaf/messaging/ipc/IPCFactory.h \
    include/thaf/messaging/ipc/IPCMessage.h \
    include/thaf/messaging/ipc/IPCMessageTrait.h \
    include/thaf/messaging/ipc/IPCReceiver.h \
    include/thaf/messaging/ipc/IPCSender.h \
    include/thaf/messaging/ipc/MessageValidator.h \
    include/thaf/messaging/ipc/cscmbk.h \
    include/thaf/messaging/ipc/cscmrs.h \
    include/thaf/messaging/ipc/prv/LocalIPCReceiver.h \
    include/thaf/messaging/ipc/prv/LocalIPCSender.h \
    include/thaf/messaging/ipc/prv/platforms/windows/LocalIPCReceiver.h \
    include/thaf/messaging/ipc/prv/platforms/windows/LocalIPCSender.h \
    include/thaf/messaging/ipc/prv/platforms/windows/NamedPipeReceiver.h \
    include/thaf/messaging/ipc/prv/platforms/windows/NamedPipeReceiverBase.h \
    include/thaf/messaging/ipc/prv/platforms/windows/NamedPipeSender.h \
    include/thaf/messaging/ipc/prv/platforms/windows/NamedPipeSenderBase.h \
    include/thaf/messaging/ipc/prv/platforms/windows/OverlappedPipeReceiver.h \
    include/thaf/messaging/ipc/prv/platforms/windows/OverlappedPipeSender.h \
    include/thaf/messaging/ipc/prv/platforms/windows/PipeShared.h


