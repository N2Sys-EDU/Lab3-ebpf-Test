add_executable(lab3_server test_utils/src/server.cpp)

add_executable(Lab3_ebpf test_utils/src/test.cpp)
target_link_libraries(Lab3_ebpf gtest gtest_main pthread)
gtest_discover_tests(Lab3_ebpf)