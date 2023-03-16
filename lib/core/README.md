# APS Networks Core Library

This library contains foundational code and utilities for APS Networks software.

## CMake Modules

CMake projects can include these by defining the following:

```
find_package(apsncore)
```

### Application Fixture

Creates a CMake fixture which launches an application during the setup phase,
and terminates it at teardown. Successfully launched fixtures will be terminated
via kill using SIGINT.

The application is provided by the `TARGET` argument, and therefore must 
previously been declared by CMake.

The fixture group for which the targets will be provided is given by 
the `FIXTURE` argument.

```
create_application_fixture(
    [SUDO]
    [TARGET target]
    [FIXTURE name]
    [ARGUMENTS list;of;args])
```


An example of it's use is running the `gearboxd` application as a fixture so
that integration tests using Python can be run in parallel.

### Doxygen

Creates a doxygen target of `doxygen_${name}`, configuring it to use the APS
Networks standard stylesheets and configuration.

```
create_doxygen_target(name
    [XML] [HTML] [LATEX]
    [TEMPLATE doxyfile_template]
    [OUTPUT_DIR path]
    [MAIN_PAGE path_to_markdown]
)
```

Targets created will be added as a dependency to a global `doxygen` target.

### Package Sources

This module addresses the absence of the ability to package sources selected by
component or sub-directory.

```cmake
create_source_package(basename-0.1.0
    INCLUDE src
    EXCLUDE **/__py_cache__
)
```

Will produce a tarball `basename-0.1.0.src.tar.gz`

## Requirements

* stdlib or boost filesystem
* `fmt::fmt`

### Documentation

* python
* sphinx (python)
* breathe (python)
* whatever sphinx theme we're using