#Look for an executable called sphinx-build
find_program(PDFLATEX_EXECUTABLE
             NAMES pdflatex
             DOC "Path to pdflatex executable")

# CMake expects the calling package to be "Sphinx", but this is embedded within
# the "apsncore" package. If we don't set this, an error will be produced
set(FPHSA_NAME_MISMATCHED YES)

include(FindPackageHandleStandardArgs)

#Handle standard arguments to find_package like REQUIRED and QUIET
find_package_handle_standard_args(pdflatex
"Failed to find pdflatex executable. Try installing with: 
    sudo apt-get install texlive-latex-base texlive-fonts-recommended \\\\ 
        texlive-fonts-extra texlive-latex-extra
"
    PDFLATEX_EXECUTABLE)