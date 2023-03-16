add_library(md5 STATIC md5.cpp)
add_library(contrib::md5 ALIAS md5)
target_include_directories(md5 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)