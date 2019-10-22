cmake_minimum_required(VERSION 3.5)

project(maf-test  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ClientServer)
add_subdirectory(Serialization)
