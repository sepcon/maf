cmake_minimum_required(VERSION 3.5)

set(MAF_SAMPLE_PJNAME maf-test)
set(MAF_SAMPLE_ROOT_DIR .)

project(${MAF_SAMPLE_PJNAME})

set(MAF_SAMPLE_BINARY_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${MAF_SAMPLE_BINARY_PATH})
set(LIBRARY_OUTPUT_PATH ${MAF_SAMPLE_BINARY_PATH})

macro(maf_add_sample sample_cpp_file_no_ext)
    add_executable(${sample_cpp_file_no_ext} "./${sample_cpp_file_no_ext}.cpp")
    target_link_libraries(${sample_cpp_file_no_ext}  maf)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message("Compiler is ${CMAKE_CXX_COMPILER_ID} then, must link ${sample_cpp_file_no_ext}  with pthread library")
        target_link_libraries(${sample_cpp_file_no_ext}  pthread)
    endif()
endmacro(maf_add_sample)

maf_add_sample(inapp-client-server)
maf_add_sample(ipc-client)
maf_add_sample(ipc-server)
maf_add_sample(serialization)
maf_add_sample(simple-stub-proxy)
