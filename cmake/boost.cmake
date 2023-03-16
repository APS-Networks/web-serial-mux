include(FetchContent)

set(BOOST_INCLUDE_LIBRARIES 
    asio
    beast
    program_options)

set(BOOST_ENABLE_CMAKE ON)

set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(build_boost SYSTEM
    GIT_REPOSITORY https://github.com/boostorg/boost/
    GIT_TAG boost-1.81.0
    GIT_PROGRESS TRUE
)

# FetchContent_MakeAvailable(boost)
FetchContent_GetProperties(build_boost)
if(NOT build_boost_POPULATED)
    FetchContent_Populate(build_boost)
    add_subdirectory(
        ${build_boost_SOURCE_DIR}
        ${build_boost_BINARY_DIR}
        EXCLUDE_FROM_ALL
        )
endif()
