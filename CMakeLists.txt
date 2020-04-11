cmake_minimum_required(VERSION 3.5)

project(maf LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MAF_ROOT_DIR .)
set(MAF_LIB_INCLUDE_PATH ${MAF_ROOT_DIR}/include)

include_directories(${MAF_LIB_INCLUDE_PATH})
add_definitions(-DMAF_MIN_ALLOWED_LOG_LEVEL=1)
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

if(BUILD_SHARED_LIBS)
    add_definitions(-DMAF_SHARED_LIBRARY)
else()
    add_definitions(-DMAF_STATIC_LIBRARY)
endif()

# <-- platforms specific sources/headers

# Add maf.lib
set( MAF_ALL_SOURCES
    ${MAF_LIBRARY_SOURCES}
    ${MAF_PLATFORM_SOURCES}
    ${MAF_LIBRARY_HEADERS}
    ${MAF_LIBRARY_PRIV_HEADERS}
    ${MAF_PLATFORM_HEADERS}
    )

set(MAF_BINARY_DIR ${CMAKE_BINARY_DIR})

if(BUILD_SHARED_LIBS)
    add_library(maf SHARED ${MAF_ALL_SOURCES})
else()
    add_library(maf STATIC ${MAF_ALL_SOURCES})
endif(BUILD_SHARED_LIBS)

if(
    ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"
    OR
    {CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"
    )
    target_link_libraries(maf pthread)
endif()


# specify variable MAF_BUILD_SAMPLE to disable building test programs
if(MAF_BUILD_SAMPLE)
    add_subdirectory("sample/")
endif(MAF_BUILD_SAMPLE)

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
endif(MSVC)

# Grouping sources/headers into tree layout for IDE browsing
if(NOT DISABLE_SOURCE_GROUP)
    source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Sources" FILES ${MAF_LIBRARY_SOURCES} )
    source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Sources" FILES ${MAF_PLATFORM_SOURCES} )
    source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Sources" FILES ${MAF_LIBRARY_PRIV_HEADERS} )
    source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Sources" FILES ${MAF_PLATFORM_HEADERS} )
    source_group(TREE ${PROJECT_SOURCE_DIR}/include PREFIX "include" FILES ${MAF_LIBRARY_HEADERS} )
endif(NOT DISABLE_SOURCE_GROUP)







