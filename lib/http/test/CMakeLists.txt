add_executable(test_http 
    test_request.cpp
    test_traits.cpp)
target_link_libraries(test_http PRIVATE apsnhttp gtest_main)
add_test(test_http test_http)