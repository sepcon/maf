cmake_minimum_required(VERSION 3.5)

set(CLIENT_SERVER_PROJECT_NAME client-server-test )
set(IPC_CLIENT_PROJECT_NAME ipc-client-test)
set(IPC_SERVER_PROJECT_NAME ipc-server-test)

if(MAF_BINARY_DIR)
    set(CMAKE_BINARY_DIR ${MAF_BINARY_DIR})
endif(MAF_BINARY_DIR)

message("Set ${MAF_BINARY_DIR} as binary dir")

set(TEST_PROJECTS
    ${CLIENT_SERVER_PROJECT_NAME}
    ${IPC_CLIENT_PROJECT_NAME}
    ${IPC_SERVER_PROJECT_NAME}
    TestServer
    TestClient
    )


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${MAF_LIB_INCLUDE_PATH})

add_executable(${CLIENT_SERVER_PROJECT_NAME}
    main.cpp

    Client.h
    Server.h
    Contract.h
    )

add_library(TestClient SHARED
    TestClient.cpp
    )

add_library(TestServer SHARED
    TestServer.cpp
    )


set(LIBRARY_OUTPUT_PATH ${MAF_TEST_BINARY_PATH}/plugin)

add_executable(${IPC_CLIENT_PROJECT_NAME}
    IPCClient/main.cpp
    )

add_executable(${IPC_SERVER_PROJECT_NAME}
    IPCServer/main.cpp
    )


foreach(PRJ_ ${TEST_PROJECTS})
    target_link_libraries(${PRJ_} maf)

    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        target_link_libraries(${PRJ_} pthread)
    endif()

    set_target_properties(${PRJ_}
        PROPERTIES
        FOLDER ${MAF_TEST_FOLDER_FILTER})
endforeach()

