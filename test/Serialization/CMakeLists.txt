cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME serialization-test)
project(${PROJECT_NAME}  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} maf)
