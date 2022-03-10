cmake_minimum_required(VERSION 3.5)



set(MAF_TEST_PJNAME maf-test)
set(MAF_TEST_ROOT_DIR .)

project(${MAF_TEST_PJNAME})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MAF_TEST_BINARY_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${MAF_TEST_BINARY_PATH})
set(LIBRARY_OUTPUT_PATH ${MAF_TEST_BINARY_PATH})

add_library(catch2 catch/catch_amalgamated.cpp)
add_library(precomp precompile.cpp)
target_precompile_headers( precomp
    PUBLIC
    <iostream>
    <maf/ITCProxy.h>
    <maf/ITCStub.h>
    <maf/LocalIPCProxy.h>
    <maf/LocalIPCStub.h>
    <maf/Messaging.h>
    <maf/logging/Logger.h>
    <maf/messaging/Processor.h>
    <maf/messaging/ProcessorEx.h>
    <maf/messaging/Routing.h>
    <maf/messaging/SignalTimer.h>
    <maf/messaging/Timer.h>
    <maf/messaging/client-server/CSContractDefinesBegin.mc.h>
    <maf/messaging/client-server/CSContractDefinesEnd.mc.h>
    <maf/messaging/client-server/ServiceStatusSignal.h>
    <maf/threading/AtomicObject.h>
    <maf/threading/MutexRef.h>
    <maf/utils/DirectExecutor.h>
    <maf/utils/Observable.h>
    <maf/utils/SignalSlots.h>
    <maf/utils/StringifyableEnum.h>
    <maf/utils/TimeMeasurement.h>
    <maf/utils/cppextension/AggregateCompare.h>
    <maf/utils/cppextension/TypeTraits.h>
    <maf/utils/serialization/AggregateDump.h>
    <maf/utils/serialization/Dumper.h>
    <maf/utils/serialization/SerializableObjectBegin.mc.h>
    <maf/utils/serialization/SerializableObjectEnd.mc.h>
    <map>
    <mutex>
    <set>
    <sstream>
    <string>
    <thread>
    <vector>
    "catch/catch_amalgamated.hpp"
    )

macro(maf_add_test test_name)
    set(the_test_binary "${test_name}_test")
    add_executable(${the_test_binary} "./${the_test_binary}.cpp")
    target_precompile_headers(${the_test_binary} REUSE_FROM precomp)
    target_link_libraries(${the_test_binary} maf catch2)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message("Compiler is ${CMAKE_CXX_COMPILER_ID} then, must link ${the_test_binary}  with pthread library")
        target_link_libraries(${the_test_binary}  pthread )
    endif()

    add_test(NAME "${test_name}.test" COMMAND ${the_test_binary})
endmacro(maf_add_test)



maf_add_test(processor)
maf_add_test(timer)
maf_add_test(serialization)
maf_add_test(stub_proxy)
maf_add_test(message_routing)
maf_add_test(ipc_sender_receiver)
maf_add_test(signal_slot)
maf_add_test(utils)

