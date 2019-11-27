cmake_minimum_required(VERSION 3.5)

project(maf LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MAF_ROOT_DIR .)
set(MAF_LIB_INCLUDE_PATH ${MAF_ROOT_DIR}/include)

include_directories(${MAF_LIB_INCLUDE_PATH})

# --> Collect platforms dependent sources/headers
file(GLOB_RECURSE MAF_LIBRARY_SOURCES
    src/maf/*.cpp)
file(GLOB_RECURSE MAF_LIBRARY_HEADERS
    include/*.h)
file(GLOB_RECURSE MAF_LIBRARY_PRIV_HEADERS
    src/maf/*.h)
# <-- platforms dependent sources/headers


# --> Collect platforms specific sources/headers
if(UNIX)
    set(MAF_PLATFORM_NAME "unix")
elseif(WIN32)
    set(MAF_PLATFORM_NAME "windows")
else()
    message(ERROR "Could not detect platform")
endif()

if(MAF_PLATFORM_NAME)
    set(MAF_PLATFORM_DIR "src/platforms/${MAF_PLATFORM_NAME}")

    file(GLOB MAF_PLATFORM_HEADERS
        "${MAF_PLATFORM_DIR}/internal/*.h"
        )
    file(GLOB MAF_PLATFORM_SOURCES
        "${MAF_PLATFORM_DIR}/internal/*.cpp"
        )
    include_directories(${MAF_PLATFORM_DIR})

endif(MAF_PLATFORM_NAME)

if(MAF_ENABLE_SHARE)
    add_definitions(-DMAF_SHARED_LIBRARY)
else()
    add_definitions(-DMAF_STATIC_LIBRARY)
endif()

# <-- platforms specific sources/headers

# Add maf.lib
add_library(maf_objlib OBJECT
    ${MAF_LIBRARY_SOURCES}
    ${MAF_PLATFORM_SOURCES}
    ${MAF_LIBRARY_HEADERS}
    ${MAF_LIBRARY_PRIV_HEADERS}
    ${MAF_PLATFORM_HEADERS}
    )

set(MAF_BINARY_DIR ${CMAKE_BINARY_DIR})

set(MAFLIB maf)

if(MAF_ENABLE_SHARE)
    set_property(TARGET maf_objlib PROPERTY POSITION_INDEPENDENT_CODE 1)
    add_library(maf SHARED $<TARGET_OBJECTS:maf_objlib>)
else()
    add_library(maf STATIC $<TARGET_OBJECTS:maf_objlib>)
endif(MAF_ENABLE_SHARE)


# specify variable MAF_ENABLE_TEST to disable building test programs
if(MAF_ENABLE_TEST)
    set(MAF_TEST_FOLDER_FILTER "Tests")
    add_subdirectory(test)
endif(MAF_ENABLE_TEST)


# Grouping sources/headers into tree layout for IDE browsing
source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Sources" FILES ${MAF_LIBRARY_SOURCES} )
source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Sources" FILES ${MAF_PLATFORM_SOURCES} )
source_group(TREE ${PROJECT_SOURCE_DIR}/include PREFIX "Headers" FILES ${MAF_LIBRARY_HEADERS} )
source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Headers" FILES ${MAF_LIBRARY_PRIV_HEADERS} )
source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Headers" FILES ${MAF_PLATFORM_HEADERS} )







