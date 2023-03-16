#! /bin/sed -nf

# Mermaid Markdown Block Replacement
# ==================================
#
# Doxygen has support for codeblocks, but does not recognise mermaid blocks
# which are rendered on GitHub. In order to have mermaid graphs working in
# GitHub as well as Doxygen, we need to transform the code block into a HTML
# `pre` block.
#
# The Doxygen header must have mermaid.js included and initialised.
#
# To use, set the following in your Doxyfile:
#
#   FILTER_PATTERNS        = *.md="sed -f ./mermaid-md.sed"


/^```mermaid\s*/{                       # Line starts with mermaid block
        s/.*/<pre class="mermaid">/     # Replace this line with HTML
        :b                              # Label
        n                               # Pull in next line
        /^```.*$/!bb                    # If line doesn't start with ```, goto
                                        # :b label
        s/.*/<\/pre>/                   # Else replace with HTML
    }