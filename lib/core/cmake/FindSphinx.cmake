#Look for an executable called sphinx-build
find_program(SPHINX_EXECUTABLE
             NAMES sphinx-build
             DOC "Path to sphinx-build executable"
)

# CMake expects the calling package to be "Sphinx", but this is embedded within
# the "apsncore" package. If we don't set this, an error will be produced
set(FPHSA_NAME_MISMATCHED YES)

include(FindPackageHandleStandardArgs)


#Handle standard arguments to find_package like REQUIRED and QUIET
find_package_handle_standard_args(Sphinx
                                  "Failed to find sphinx-build executable"
                                  SPHINX_EXECUTABLE)