cmake_minimum_required(VERSION 3.5)

set(MAF_TEST_CLIENT_SERVER_PROJECT_NAME client-server-test )
project(${MAF_TEST_CLIENT_SERVER_PROJECT_NAME}  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${MAF_LIB_INCLUDE_PATH})
add_executable(${MAF_TEST_CLIENT_SERVER_PROJECT_NAME}
    main.cpp

    Client.h
    Server.h
    Contract.h
    )
target_link_libraries(${MAF_TEST_CLIENT_SERVER_PROJECT_NAME} maf)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_link_libraries(maftest pthread)
endif()
