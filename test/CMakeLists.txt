cmake_minimum_required(VERSION 3.5)

set(MAF_TEST_PJNAME maf-test)
set(MAF_TEST_ROOT_DIR .)

project(${MAF_TEST_PJNAME})

set(MAF_TEST_BINARY_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${MAF_TEST_BINARY_PATH})
set(LIBRARY_OUTPUT_PATH ${MAF_TEST_BINARY_PATH})

macro(maf_add_test test_name)
    set(the_test_binary "${test_name}_test")
    add_executable(${the_test_binary} "./${the_test_binary}.cpp")
    target_link_libraries(${the_test_binary} maf)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message("Compiler is ${CMAKE_CXX_COMPILER_ID} then, must link ${the_test_binary}  with pthread library")
        target_link_libraries(${the_test_binary}  pthread)
    endif()

    add_test(NAME "${test_name}.test" COMMAND ${the_test_binary})

    add_custom_command(TARGET ${the_test_binary} POST_BUILD
                       COMMAND ${the_test_binary})
endmacro(maf_add_test)

maf_add_test(component)
maf_add_test(serialization)
maf_add_test(stub_proxy)
maf_add_test(message_routing)
maf_add_test(ipc_sender_receiver)


