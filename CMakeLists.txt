cmake_minimum_required(VERSION 3.5)

project(maf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_library(maf

    # Source files
    src/maf/messaging/Component.cpp
    src/maf/messaging/CompThread.cpp
    src/maf/messaging/MessageBase.cpp
    src/maf/messaging/Timer.cpp
    src/maf/messaging/client-server/Address.cpp
    src/maf/messaging/client-server/CSMessage.cpp
    src/maf/messaging/client-server/ClientBase.cpp
    src/maf/messaging/client-server/IAMessageRouter.cpp
    src/maf/messaging/client-server/RequestKeeper.cpp
    src/maf/messaging/client-server/ServerBase.cpp
    src/maf/messaging/client-server/ServiceProxyBase.cpp
    src/maf/messaging/client-server/ServiceStubBase.cpp
    src/maf/messaging/client-server/ServiceStubBaseImpl.cpp
    src/maf/messaging/client-server/ServiceProxyBaseImpl.cpp
    src/maf/messaging/client-server/ipc/BytesCommunicator.cpp
    src/maf/messaging/client-server/ipc/IPCClientBase.cpp
    src/maf/messaging/client-server/ipc/IPCServerBase.cpp
    src/maf/messaging/client-server/ipc/LocalIPCReceiver.cpp
    src/maf/messaging/client-server/ipc/LocalIPCSender.cpp
    src/maf/messaging/client-server/ipc/IPCFactory.cpp
    src/maf/messaging/client-server/ipc/IPCMessage.cpp
    src/maf/messaging/client-server/ipc/IPCReceiver.cpp
    src/maf/threading/Thread.cpp
    src/maf/threading/threadpool/DynamicCountThreadPool.cpp
    src/maf/threading/threadpool/ThreadPoolFactory.cpp
    src/maf/threading/threadpool/PriorityThreadPool.cpp
    src/maf/threading/threadpool/StableThreadPool.cpp
    src/maf/threading/time/TimerManager.cpp
    src/maf/threading/time/TimerManagerImpl.cpp
    src/maf/threading/time/Waiter.cpp
    src/maf/utils/IDManager.cpp
    src/maf/utils/serialization/Serializer.cpp


    # Header files

    include/maf/messaging/BasicMessages.h
    include/maf/messaging/Component.h
    include/maf/messaging/ExtensibleComponent.h
    include/maf/messaging/CompThread.h
    include/maf/messaging/MessageBase.h
    include/maf/messaging/MessageQueue.h
    include/maf/messaging/MsgDefHelper.mc.h
    include/maf/messaging/Timer.h
    include/maf/messaging/Communicator.h
    include/maf/messaging/client-server/ClientBase.h
    include/maf/messaging/client-server/IAMessageRouter.h
    include/maf/messaging/client-server/IAServiceProxy.h
    include/maf/messaging/client-server/IAServiceStub.h
    include/maf/messaging/client-server/QueueingServiceProxy.h
    include/maf/messaging/client-server/QueueingServiceProxy.impl.h
    include/maf/messaging/client-server/QueueingServiceStub.h
    include/maf/messaging/client-server/SCQServiceProxy.h
    include/maf/messaging/client-server/SSQServiceStub.h
    include/maf/messaging/client-server/ServerBase.h
    include/maf/messaging/client-server/ServiceProxyBase.h
    include/maf/messaging/client-server/ServiceStubBase.h
    include/maf/messaging/client-server/Address.h
    include/maf/messaging/client-server/CSDefines.h
    include/maf/messaging/client-server/CSMessage.h
    include/maf/messaging/client-server/CSMessageReceiver.h
    include/maf/messaging/client-server/CSStatus.h
    include/maf/messaging/client-server/CSTypes.h
    include/maf/messaging/client-server/ClientInterface.h
    include/maf/messaging/client-server/RegID.h
    include/maf/messaging/client-server/RequestKeeper.h
    include/maf/messaging/client-server/ServerInterface.h
    include/maf/messaging/client-server/ServiceMessageReceiver.h
    include/maf/messaging/client-server/ServiceProviderInterface.h
    include/maf/messaging/client-server/ServiceProxyInterface.h
    include/maf/messaging/client-server/ServiceRequesterInterface.h
    include/maf/messaging/client-server/ServiceStatusObserverInterface.h
    include/maf/messaging/client-server/ServiceStubHandlerInterface.h
    include/maf/messaging/client-server/ServiceStubInterface.h
    include/maf/messaging/client-server/internal/CSShared.h
    include/maf/messaging/client-server/IAMessageTrait.h
    include/maf/messaging/client-server/Connection.h
    include/maf/messaging/client-server/CSContractBegin.mc.h
    include/maf/messaging/client-server/CSContractDefines.mc.h
    include/maf/messaging/client-server/CSContractEnd.mc.h
    include/maf/messaging/client-server/ipc/BytesCommunicator.h
    include/maf/messaging/client-server/ipc/IPCClientBase.h
    include/maf/messaging/client-server/ipc/IPCServerBase.h
    include/maf/messaging/client-server/ipc/IPCFactory.h
    include/maf/messaging/client-server/ipc/IPCMessage.h
    include/maf/messaging/client-server/ipc/IPCMessageTrait.h
    include/maf/messaging/client-server/ipc/IPCReceiver.h
    include/maf/messaging/client-server/ipc/IPCSender.h
    include/maf/messaging/client-server/ipc/IPCTypes.h
    include/maf/messaging/client-server/ipc/LocalIPCClient.h
    include/maf/messaging/client-server/ipc/LocalIPCServer.h
    include/maf/messaging/client-server/ipc/LocalIPCServiceProxy.h
    include/maf/messaging/client-server/ipc/LocalIPCServiceStub.h
    include/maf/messaging/client-server/ipc/MessageValidator.h
    include/maf/messaging/MessageHandler.h
    include/maf/threading/IThreadPool.h
    include/maf/threading/ThreadPoolImplBase.h
    include/maf/threading/Queue.h
    include/maf/threading/Runnable.h
    include/maf/threading/Signal.h
    include/maf/threading/Thread.h
    include/maf/threading/ThreadJoiner.h
    include/maf/threading/ThreadPoolFactory.h
    include/maf/threading/ThreadSafeQueue.h
    include/maf/threading/TimerManager.h
    include/maf/threading/Waiter.h
    include/maf/utils/TimeMeasurement.h
    include/maf/utils/cppextension/Loop.mc.h
    include/maf/utils/cppextension/Maf.mc.h
    include/maf/utils/cppextension/SyncObject.h
    include/maf/utils/debugging/Debug.h
    include/maf/utils/IDManager.h
    include/maf/patterns/Patterns.h
    include/maf/utils/cppextension/TupleManip.h
    include/maf/utils/cppextension/TypeTraits.h
    include/maf/utils/serialization/3rdparty/Json11Trait.h
    include/maf/utils/serialization/BasicTypes.h
    include/maf/utils/serialization/ByteArray.h
    include/maf/utils/serialization/DumpHelper.h
    include/maf/utils/serialization/JsonTrait.h
    include/maf/utils/serialization/SerializableInterface.h
    include/maf/utils/serialization/SerializationTrait.h
    include/maf/utils/serialization/Serializer.h
    include/maf/utils/serialization/Tplkdef.mc.h
    include/maf/utils/serialization/TupleLikeObject.mc.h
    src/maf/messaging/client-server/ServiceStubBaseImpl.h
    src/maf/messaging/client-server/ServiceProxyBaseImpl.h
    src/maf/threading/threadpool/DynamicCountThreadPool.h
    src/maf/threading/threadpool/PriorityThreadPool.h
    src/maf/threading/threadpool/StableThreadPool.h
    src/maf/messaging/client-server/ipc/platforms/windows/NamedPipeReceiverBase.h
    src/maf/messaging/client-server/ipc/platforms/windows/NamedPipeSenderBase.h
    src/maf/messaging/client-server/ipc/platforms/windows/PipeShared.h)

add_executable(maftest src/maf/main.cpp)
target_link_libraries(maftest maf pthread)

include_directories(include/)
