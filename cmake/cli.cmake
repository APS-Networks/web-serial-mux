include(FetchContent)
FetchContent_Declare(daniellecli SYSTEM
    GIT_REPOSITORY  https://github.com/daniele77/cli.git
    GIT_TAG         v2.0.0
    GIT_PROGRESS    TRUE
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/cli
        -DCLI_BuildExamples=OFF
    UPDATE_COMMAND ""
    PATCH_COMMAND patch -p1 < ${PROJECT_SOURCE_DIR}/patches/0000-make-cli-open.patch
)
FetchContent_MakeAvailable(daniellecli)
