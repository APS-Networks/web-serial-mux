add_executable(test_core 
    test_ansi.cpp
    test_logging.cpp
    test_result.cpp)
target_link_libraries(test_core PRIVATE apsncore gtest_main)
add_test(test_core test_core)