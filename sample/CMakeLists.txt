cmake_minimum_required(VERSION 3.5)

set(MAF_SAMPLE_PJNAME maf-test)
set(MAF_SAMPLE_ROOT_DIR .)

project(${MAF_SAMPLE_PJNAME})

set(MAF_SAMPLE_BINARY_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${MAF_SAMPLE_BINARY_PATH})
set(LIBRARY_OUTPUT_PATH ${MAF_SAMPLE_BINARY_PATH})

macro(maf_add_sample sample_cpp_file_no_ext)
    set(sample_target_name "${sample_cpp_file_no_ext}_sample")
    add_executable(${sample_target_name} "./${sample_target_name}.cpp")
    target_link_libraries(${sample_target_name}  maf)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message("Compiler is ${CMAKE_CXX_COMPILER_ID} then, must link ${sample_target_name}  with pthread library")
        target_link_libraries(${sample_target_name}  pthread)
    endif()
endmacro(maf_add_sample)

macro(maf_add_test test_name)
    add_executable(${test_name} "./${test_name}.cpp")
    target_link_libraries(${test_name} maf)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message("Compiler is ${CMAKE_CXX_COMPILER_ID} then, must link ${sample_cpp_file_no_ext}  with pthread library")
        target_link_libraries(${sample_cpp_file_no_ext}  pthread)
    endif()

    add_test(NAME "${test_name}_test" COMMAND ${test_name})
endmacro(maf_add_test)

maf_add_sample(inapp-client-server)
maf_add_sample(ipc-client)
maf_add_sample(ipc-server)
maf_add_sample(simple-stub-proxy)
