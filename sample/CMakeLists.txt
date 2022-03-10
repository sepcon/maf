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

maf_add_sample(inapp-client-server)
maf_add_sample(ipc-client)
maf_add_sample(ipc-server)
maf_add_sample(simple-stub-proxy)
maf_add_sample(processor)
maf_add_sample(timer)
maf_add_sample(signal_slot)
