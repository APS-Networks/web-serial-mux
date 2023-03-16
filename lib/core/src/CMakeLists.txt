add_library(apsncore STATIC 
    ansi.cpp
    fmt.cpp
    lock.cpp
    logging.cpp
    result.cpp
    utility.cpp
    detail/result.cpp
)

add_library(apsn::core ALIAS apsncore)

target_include_directories(apsncore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/apsn
)
target_compile_options(apsncore PUBLIC
    -Wall
    -Werror
    -Wextra
    -pedantic)
target_compile_features(apsncore PUBLIC cxx_std_20)

target_link_libraries(apsncore PUBLIC fmt::fmt)

target_precompile_headers(apsncore PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/apsn/debug.hpp)