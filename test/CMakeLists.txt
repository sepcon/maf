cmake_minimum_required(VERSION 3.5)

set(MAF_TEST_PJNAME maf-test)
set(MAF_TEST_ROOT_DIR .)

project(${MAF_TEST_PJNAME})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_subdirectory(ClientServer)
add_subdirectory(Serialization)

source_group(TREE ${PROJECT_SOURCE_DIR}/test PREFIX "Tests")
