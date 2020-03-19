cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(MAF_BINARY_DIR)
    set(CMAKE_BINARY_DIR ${MAF_BINARY_DIR})
endif(MAF_BINARY_DIR)

message("Set ${MAF_BINARY_DIR} as binary dir")

include_directories(${MAF_LIB_INCLUDE_PATH})

add_executable(component-test
    main.cpp
    )

target_link_libraries(component-test maf)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_link_libraries(component-test pthread)
endif()

set_target_properties(
    component-test
    PROPERTIES
    FOLDER ${MAF_TEST_FOLDER_FILTER}
    )


