
TARGET = maf
TEMPLATE = lib
CONFIG += staticlib

QT       -= core gui

#TARGET = maf
#TEMPLATE = app

#QT       -= core gui

CONFIG += c++17

DEFINES += ENABLE_IPC_DEBUG=2

#DEFINES += MESSAGING_BY_PRIORITY

win32: {
    INCLUDEPATH += $(VC_IncludePath) $(WindowsSDK_IncludePath)
    LIBS += -lKernel32

    mingw {
        QMAKE_CXXFLAGS += -std=c++17 -O0
        LIBS += -lpthread
    }
    msvc {
        QMAKE_CXXFLAGS += /std:c++17
    }

} else {
    QMAKE_CXXFLAGS += -std=c++17
    LIBS += -lpthread
}


INCLUDEPATH += ./include

#    src/maf/main.cpp \
SOURCES += \
    src/maf/messaging/Component.cpp \
    src/maf/messaging/Timer.cpp \
    src/maf/logging/ConsoleLogger.cpp \
    src/maf/logging/LoggingComponent.cpp \
    src/maf/messaging/client-server/CSMessage.cpp \
    src/maf/messaging/client-server/ClientBase.cpp \
    src/maf/messaging/client-server/IAMessageRouter.cpp \
    src/maf/messaging/client-server/RequestKeeper.cpp \
    src/maf/messaging/client-server/ServerBase.cpp \
    src/maf/messaging/client-server/ServiceProxyBase.cpp \
    src/maf/messaging/client-server/ServiceStubBase.cpp \
    src/maf/messaging/client-server/Address.cpp \
    src/maf/messaging/client-server/ipc/BytesCommunicator.cpp \
    src/maf/messaging/client-server/ipc/IPCClientBase.cpp \
    src/maf/messaging/client-server/ipc/IPCServerBase.cpp \
    src/maf/messaging/client-server/ipc/LocalIPCReceiver.cpp \
    src/maf/messaging/client-server/ipc/LocalIPCSender.cpp \
    src/maf/threading/Thread.cpp \
    src/maf/threading/threadpool/DynamicCountThreadPool.cpp \
    src/maf/threading/threadpool/ThreadPoolFactory.cpp \
    src/maf/threading/threadpool/PriorityThreadPool.cpp \
    src/maf/threading/threadpool/StableThreadPool.cpp \
    src/maf/threading/time/TimerManager.cpp \
    src/maf/threading/time/TimerManagerImpl.cpp \
    src/maf/threading/time/Waiter.cpp \
    src/maf/utils/IDManager.cpp \
    src/maf/utils/serialization/Serializer.cpp \
    src/maf/messaging/client-server/ipc/IPCFactory.cpp \
    src/maf/messaging/client-server/ipc/IPCMessage.cpp \
    src/maf/messaging/client-server/ipc/IPCReceiver.cpp

HEADERS += \
    include/maf/messaging/BasicMessages.h \
    include/maf/messaging/Component.h \
    include/maf/messaging/LocalIPCServer.h \
    include/maf/messaging/MessageBase.h \
    include/maf/messaging/MessageQueue.h \
    include/maf/messaging/MsgDefHelper.mc.h \
    include/maf/messaging/Timer.h \
    include/maf/logging/ConsoleLogger.h \
    include/maf/logging/LoggerBase.h \
    include/maf/logging/LoggerInterface.h \
    include/maf/logging/LoggingComponent.h \
    include/maf/messaging/Communicator.h \
    include/maf/messaging/client-server/ClientBase.h \
    include/maf/messaging/client-server/IAMessageRouter.h \
    include/maf/messaging/client-server/IAServiceProxy.h \
    include/maf/messaging/client-server/IAServiceStub.h \
    include/maf/messaging/client-server/QueueingServiceProxy.h \
    include/maf/messaging/client-server/QueueingServiceStub.h \
    include/maf/messaging/client-server/SCQServiceProxy.h \
    include/maf/messaging/client-server/SSQServiceStub.h \
    include/maf/messaging/client-server/ServerBase.h \
    include/maf/messaging/client-server/ServerDomainController.h \
    include/maf/messaging/client-server/ServiceProxyBase.h \
    include/maf/messaging/client-server/ServiceStubBase.h \
    include/maf/messaging/client-server/Address.h \
    include/maf/messaging/client-server/CSDefines.h \
    include/maf/messaging/client-server/CSMessage.h \
    include/maf/messaging/client-server/CSMessageReceiver.h \
    include/maf/messaging/client-server/CSStatus.h \
    include/maf/messaging/client-server/CSTypes.h \
    include/maf/messaging/client-server/ClientInterface.h \
    include/maf/messaging/client-server/RegisterDataStructure.h \
    include/maf/messaging/client-server/RequestKeeper.h \
    include/maf/messaging/client-server/ServerInterface.h \
    include/maf/messaging/client-server/ServiceMessageReceiver.h \
    include/maf/messaging/client-server/ServiceProviderInterface.h \
    include/maf/messaging/client-server/ServiceProxyInterface.h \
    include/maf/messaging/client-server/ServiceRequesterInterface.h \
    include/maf/messaging/client-server/ServiceStatusObserverInterface.h \
    include/maf/messaging/client-server/ServiceStubHandlerInterface.h \
    include/maf/messaging/client-server/ServiceStubInterface.h \
    include/maf/messaging/client-server/internal/CSShared.h \
    include/maf/messaging/client-server/IAMessageTrait.h \
    include/maf/messaging/client-server/Connection.h \
    include/maf/messaging/client-server/CSContractBegin.mc.h \
    include/maf/messaging/client-server/CSContractDefines.mc.h \
    include/maf/messaging/client-server/CSContractEnd.mc.h \
    include/maf/messaging/client-server/ipc/BytesCommunicator.h \
    include/maf/messaging/client-server/ipc/IPCClientBase.h \
    include/maf/messaging/client-server/ipc/IPCServerBase.h \
    include/maf/messaging/client-server/ipc/IPCTypes.h \
    include/maf/messaging/client-server/ipc/LocalIPCClient.h \
    include/maf/messaging/client-server/ipc/LocalIPCServer.h \
    include/maf/messaging/client-server/ipc/LocalIPCServiceProxy.h \
    include/maf/messaging/client-server/ipc/LocalIPCServiceStub.h \
    include/maf/messaging/client-server/ipc/internal/platforms/linux/LocalIPCSender.h \
    include/maf/messaging/MessageHandler.h \
    include/maf/messaging/client-server/ipc/internal/platforms/windows/LocalIPCReceiverImpl.h \
    include/maf/messaging/client-server/ipc/internal/platforms/windows/LocalIPCSenderImpl.h \
    include/maf/threading/IThreadPool.h \
    include/maf/threading/Queue.h \
    include/maf/threading/Runnable.h \
    include/maf/threading/Signal.h \
    include/maf/threading/Thread.h \
    include/maf/threading/ThreadJoiner.h \
    include/maf/threading/ThreadPoolFactory.h \
    include/maf/threading/ThreadSafeQueue.h \
    include/maf/threading/TimerManager.h \
    include/maf/threading/Waiter.h \
    include/maf/threading/internal/DynamicCountThreadPool.h \
    include/maf/threading/internal/PriorityThreadPool.h \
    include/maf/threading/internal/StableThreadPool.h \
    include/maf/threading/internal/ThreadPoolImplBase.h \
    include/maf/threading/internal/TimerManagerImpl.h \
    include/maf/utils/TimeMeasurement.h \
    include/maf/utils/cppextension/Loop.mc.h \
    include/maf/utils/cppextension/SyncObject.h \
    include/maf/utils/cppextension/maf.mc.h \
    include/maf/utils/debugging/Debug.h \
    include/maf/utils/IDManager.h \
    include/maf/patterns/Patterns.h \
    include/maf/utils/cppextension/TupleManip.h \
    include/maf/utils/cppextension/TypeTraits.h \
    include/maf/utils/serialization/ByteArray.h \
    include/maf/utils/serialization/DumpHelper.h \
    include/maf/utils/serialization/JsonTrait.h \
    include/maf/utils/serialization/SBObjDef.mc.h \
    include/maf/utils/serialization/SBObjectDefHlp.mc.h \
    include/maf/utils/serialization/SerializableInterface.h \
    include/maf/utils/serialization/SerializationTrait.h \
    include/maf/utils/serialization/Serializer.h \
    include/maf/messaging/client-server/ipc/IPCFactory.h \
    include/maf/messaging/client-server/ipc/IPCMessage.h \
    include/maf/messaging/client-server/ipc/IPCMessageTrait.h \
    include/maf/messaging/client-server/ipc/IPCReceiver.h \
    include/maf/messaging/client-server/ipc/IPCSender.h \
    include/maf/messaging/client-server/ipc/MessageValidator.h \
    include/maf/messaging/client-server/ipc/internal/LocalIPCReceiver.h \
    include/maf/messaging/client-server/ipc/internal/LocalIPCSender.h \
    include/maf/messaging/client-server/ipc/internal/platforms/windows/NamedPipeReceiverBase.h \
    include/maf/messaging/client-server/ipc/internal/platforms/windows/NamedPipeSenderBase.h \
    include/maf/messaging/client-server/ipc/internal/platforms/windows/PipeShared.h

DISTFILES +=


