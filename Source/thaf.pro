
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

    SOURCES +=


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


INCLUDEPATH += ./include

SOURCES += \
    src/thaf/main.cpp \
    src/thaf/messaging/Component.cpp \
    src/thaf/messaging/Timer.cpp \
    src/thaf/logging/ConsoleLogger.cpp \
    src/thaf/logging/LoggingComponent.cpp \
    src/thaf/messaging/client-server/CSMessage.cpp \
    src/thaf/messaging/client-server/ClientBase.cpp \
    src/thaf/messaging/client-server/DomainController.cpp \
    src/thaf/messaging/client-server/IAMessageRouter.cpp \
    src/thaf/messaging/client-server/RequestKeeper.cpp \
    src/thaf/messaging/client-server/ServerBase.cpp \
    src/thaf/messaging/client-server/ServiceProxyBase.cpp \
    src/thaf/messaging/client-server/ServiceStubBase.cpp \
    src/thaf/messaging/client-server/Address.cpp \
    src/thaf/messaging/client-server/ipc/BytesCommunicator.cpp \
    src/thaf/messaging/client-server/ipc/IPCClientBase.cpp \
    src/thaf/messaging/client-server/ipc/IPCServerBase.cpp \
    src/thaf/messaging/client-server/ipc/LocalIPCReceiver.cpp \
    src/thaf/messaging/client-server/ipc/LocalIPCSender.cpp \
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
    src/thaf/messaging/client-server/ipc/IPCFactory.cpp \
    src/thaf/messaging/client-server/ipc/IPCMessage.cpp \
    src/thaf/messaging/client-server/ipc/IPCReceiver.cpp

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
    include/thaf/messaging/client-server/DomainController.h \
    include/thaf/messaging/client-server/IAMessageRouter.h \
    include/thaf/messaging/client-server/IAServiceProxy.h \
    include/thaf/messaging/client-server/IAServiceStub.h \
    include/thaf/messaging/client-server/QueueingServiceProxy.h \
    include/thaf/messaging/client-server/QueueingServiceStub.h \
    include/thaf/messaging/client-server/SCQServiceProxy.h \
    include/thaf/messaging/client-server/SSQServiceStub.h \
    include/thaf/messaging/client-server/ServerBase.h \
    include/thaf/messaging/client-server/ServerDomainController.h \
    include/thaf/messaging/client-server/ServiceProxyBase.h \
    include/thaf/messaging/client-server/ServiceStubBase.h \
    include/thaf/messaging/client-server/Address.h \
    include/thaf/messaging/client-server/CSDefines.h \
    include/thaf/messaging/client-server/CSMessage.h \
    include/thaf/messaging/client-server/CSMessageReceiver.h \
    include/thaf/messaging/client-server/CSStatus.h \
    include/thaf/messaging/client-server/CSTypes.h \
    include/thaf/messaging/client-server/ClientInterface.h \
    include/thaf/messaging/client-server/DomainUser.h \
    include/thaf/messaging/client-server/RegisterDataStructure.h \
    include/thaf/messaging/client-server/RequestKeeper.h \
    include/thaf/messaging/client-server/ServerInterface.h \
    include/thaf/messaging/client-server/ServiceMessageReceiver.h \
    include/thaf/messaging/client-server/ServiceProviderInterface.h \
    include/thaf/messaging/client-server/ServiceProxyInterface.h \
    include/thaf/messaging/client-server/ServiceRequesterInterface.h \
    include/thaf/messaging/client-server/ServiceStatusObserverInterface.h \
    include/thaf/messaging/client-server/ServiceStubHandlerInterface.h \
    include/thaf/messaging/client-server/ServiceStubInterface.h \
    include/thaf/messaging/client-server/internal/CSShared.h \
    include/thaf/messaging/client-server/IAMessageTrait.h \
    include/thaf/messaging/client-server/Connection.h \
    include/thaf/messaging/client-server/ipc/BytesCommunicator.h \
    include/thaf/messaging/client-server/ipc/CSContractBegin.mc.h \
    include/thaf/messaging/client-server/ipc/CSContractDefines.mc.h \
    include/thaf/messaging/client-server/ipc/CSContractEnd.mc.h \
    include/thaf/messaging/client-server/ipc/IPCClientBase.h \
    include/thaf/messaging/client-server/ipc/IPCServerBase.h \
    include/thaf/messaging/client-server/ipc/IPCTypes.h \
    include/thaf/messaging/client-server/ipc/LocalIPCClient.h \
    include/thaf/messaging/client-server/ipc/LocalIPCServer.h \
    include/thaf/messaging/client-server/ipc/LocalIPCServiceProxy.h \
    include/thaf/messaging/client-server/ipc/LocalIPCServiceStub.h \
    include/thaf/messaging/client-server/ipc/internal/platforms/linux/LocalIPCSender.h \
    include/thaf/messaging/MessageHandler.h \
    include/thaf/messaging/client-server/ipc/internal/platforms/windows/LocalIPCReceiverImpl.h \
    include/thaf/messaging/client-server/ipc/internal/platforms/windows/LocalIPCSenderImpl.h \
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
    include/thaf/threading/internal/DynamicCountThreadPool.h \
    include/thaf/threading/internal/PriorityThreadPool.h \
    include/thaf/threading/internal/StableThreadPool.h \
    include/thaf/threading/internal/ThreadPoolImplBase.h \
    include/thaf/threading/internal/TimerManagerImpl.h \
    include/thaf/utils/TimeMeasurement.h \
    include/thaf/utils/cppextension/Loop.mc.h \
    include/thaf/utils/cppextension/SyncObject.h \
    include/thaf/utils/cppextension/thaf.mc.h \
    include/thaf/utils/debugging/Debug.h \
    include/thaf/utils/IDManager.h \
    include/thaf/patterns/Patterns.h \
    include/thaf/utils/cppextension/TupleManip.h \
    include/thaf/utils/cppextension/TypeTraits.h \
    include/thaf/utils/serialization/ByteArray.h \
    include/thaf/utils/serialization/DumpHelper.h \
    include/thaf/utils/serialization/SerializableInterface.h \
    include/thaf/utils/serialization/SerializableObject.h \
    include/thaf/utils/serialization/SerializableObject.mc.h \
    include/thaf/utils/serialization/SerializationTrait.h \
    include/thaf/utils/serialization/Serializer.h \
    include/thaf/messaging/client-server/ipc/IPCFactory.h \
    include/thaf/messaging/client-server/ipc/IPCMessage.h \
    include/thaf/messaging/client-server/ipc/IPCMessageTrait.h \
    include/thaf/messaging/client-server/ipc/IPCReceiver.h \
    include/thaf/messaging/client-server/ipc/IPCSender.h \
    include/thaf/messaging/client-server/ipc/MessageValidator.h \
    include/thaf/messaging/client-server/ipc/internal/LocalIPCReceiver.h \
    include/thaf/messaging/client-server/ipc/internal/LocalIPCSender.h \
    include/thaf/messaging/client-server/ipc/internal/platforms/windows/NamedPipeReceiverBase.h \
    include/thaf/messaging/client-server/ipc/internal/platforms/windows/NamedPipeSenderBase.h \
    include/thaf/messaging/client-server/ipc/internal/platforms/windows/PipeShared.h


