cmake_minimum_required(VERSION 3.17)
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
cmake_policy(SET CMP0077 NEW)

project(APSNCoreLibraries VERSION 1.1.1)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(ApplicationFixture)
include(PackageSources)
include(Doxygen)
include(FindSphinx)
include(Findpdflatex)

add_subdirectory(docs)
add_subdirectory(src)
add_subdirectory(test)





# include(CMakePackageConfigHelpers)
# configure_package_config_file(
#         ${CMAKE_CURRENT_SOURCE_DIR}/cmake/apsncoreConfig.cmake.in
#         ${CMAKE_CURRENT_BINARY_DIR}/apsncoreConfig.cmake
#     INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/apsncore
# )

# target_set_clang_tidy(serialmux)