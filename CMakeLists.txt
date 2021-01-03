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
file(GLOB_RECURSE MAF_COMMON_SOURCES
    src/common/maf/*.cpp)
file(GLOB_RECURSE MAF_LIBRARY_HEADERS
    include/*.h)
file(GLOB_RECURSE MAF_COMMON_PRIV_SOURCES
    src/common/maf/*.h)
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

    file(GLOB_RECURSE MAF_PLATFORM_HEADERS
        "${MAF_PLATFORM_DIR}/*.h"
        )
    file(GLOB_RECURSE MAF_PLATFORM_SOURCES
        "${MAF_PLATFORM_DIR}/*.cpp"
        )
    include_directories(${MAF_PLATFORM_DIR})
endif(MAF_PLATFORM_NAME)

if(BUILD_SHARED_LIBS)
    set(MAF_BUILD_SHARED_LIB ${BUILD_SHARED_LIBS})
endif(BUILD_SHARED_LIBS)

if(MAF_BUILD_SHARED_LIB)
    add_definitions(-DMAF_SHARED_LIBRARY)
else()
    add_definitions(-DMAF_STATIC_LIBRARY)
endif()

# <-- platforms specific sources/headers

# Add maf.lib
set( MAF_ALL_SOURCES
    ${MAF_COMMON_SOURCES}
    ${MAF_PLATFORM_SOURCES}
    ${MAF_LIBRARY_HEADERS}
    ${MAF_COMMON_PRIV_SOURCES}
    ${MAF_PLATFORM_HEADERS}
    )

set(MAF_BINARY_DIR ${CMAKE_BINARY_DIR})

if(MAF_BUILD_SHARED_LIB)
    add_library(maf SHARED ${MAF_ALL_SOURCES})
else()
    add_library(maf STATIC ${MAF_ALL_SOURCES})
endif(MAF_BUILD_SHARED_LIB)

if(
    ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"
    OR
    {CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"
    )
    target_link_libraries(maf pthread)
endif()

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()


# specify variable MAF_BUILD_SAMPLE to disable building test programs
if(MAF_BUILD_SAMPLE)
    add_subdirectory("sample/")
endif(MAF_BUILD_SAMPLE)
if(MAF_ENABLE_TEST)
    enable_testing()
    add_subdirectory("test/")
endif(MAF_ENABLE_TEST)

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    message(${CMAKE_CXX_FLAGS_RELEASE})
endif(MSVC)

# Grouping sources/headers into tree layout for IDE browsing
if(NOT DISABLE_SOURCE_GROUP)
    source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Sources" FILES ${MAF_COMMON_SOURCES} )
    source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Sources" FILES ${MAF_PLATFORM_SOURCES} )
    source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Sources" FILES ${MAF_COMMON_PRIV_SOURCES} )
    source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Sources" FILES ${MAF_PLATFORM_HEADERS} )
    source_group(TREE ${PROJECT_SOURCE_DIR}/include PREFIX "include" FILES ${MAF_LIBRARY_HEADERS} )
endif(NOT DISABLE_SOURCE_GROUP)







