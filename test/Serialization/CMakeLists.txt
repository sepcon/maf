cmake_minimum_required(VERSION 3.5)

set(SERIALIZATION_PJNAME serialization-test)

#project(${MAF_TEST_PJNAME}  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${SERIALIZATION_PJNAME} main.cpp)
target_link_libraries(${SERIALIZATION_PJNAME} maf)

set_target_properties(${SERIALIZATION_PJNAME}
    PROPERTIES
    FOLDER ${MAF_TEST_FOLDER_FILTER})
