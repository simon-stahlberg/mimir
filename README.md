# Mimir: PDDL Parser and Planner Toolkit

Mimir is a C++ package designed to facilitate the parsing and inspection of Planning Domain Definition Language (PDDL) files. With Mimir, you can extract  information from PDDL files and navigate through the parsed representation using an object-oriented approach. The package also includes a lifted successor generator, allowing you to implement your own planners with ease. Additionally, Mimir provides the functionality to expand the entire state-space, enabling convenient generation of (labeled) datasets for machine learning tasks.

## Supported PDDL Subset

Mimir can handle STRIPS planning problems with negative preconditions and typing.

## Getting Started

### Dependencies

We provide a CMake Superbuild project that takes care of downloading, building, and installing all dependencies.

```console
# Configure dependencies
cmake -S dependencies -B dependencies/build -DCMAKE_INSTALL_PREFIX=dependencies/installs
# Build and install dependencies
cmake --build dependencies/build -j16
```

### Build Instructions

```console
# Configure with installation prefixes of all dependencies
cmake -S . -B build -DCMAKE_PREFIX_PATH=${PWD}/dependencies/installs
# Build
cmake --build build -j16
# Install (optional)
cmake --install build --prefix=<path/to/installation-directory>
```

### IDE Support

We developed Loki in Visual Studio Code. We recommend installing the `C/C++` and `CMake Tools` extensions by Microsoft. To get maximum IDE support, you should set the following `Cmake: Configure Args` in the `CMake Tools` extension settings under `Workspace`:

- `-DCMAKE_PREFIX_PATH=${workspaceFolder}/dependencies/installs`

After running `CMake: Configure` in Visual Studio Code (ctrl + shift + p), you should see all include paths being correctly resolved.
