cmake_minimum_required(VERSION 3.5)

set(MAF_TEST_PJNAME maf-test)
set(MAF_TEST_ROOT_DIR .)

project(${MAF_TEST_PJNAME})

add_subdirectory(ClientServer)
add_subdirectory(Serialization)

source_group(TREE ${PROJECT_SOURCE_DIR}/test PREFIX "Tests")
