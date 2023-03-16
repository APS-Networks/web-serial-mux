include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FETCHCONTENT_QUIET OFF)
include(FetchContent)
FetchContent_Declare(googletest SYSTEM
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG v1.13.x
  GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(googletest)