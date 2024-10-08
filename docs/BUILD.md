# Building Mimir (C++)

We assume that the user has a C++ compiler and CMake set up on their machine.

## Installing the Dependencies

Mimir depends on the following set of libraries:

- [Loki](https://github.com/drexlerd/Loki) for parsing PDDL files,
- [Boost](https://www.boost.org/) header-only libraries (Fusion, Spirit x3, Container, BGL),
- [Cista](https://github.com/felixguendling/cista/) for zero-copy serialization,
- [Nauty and Traces](https://users.cecs.anu.edu.au/~bdm/nauty/) for graph isomorphism testing,
- [GoogleBenchmark](https://github.com/google/benchmark) for automated performance benchmarking, and
- [GoogleTest](https://github.com/google/googletest) for unit testing.

Run the following sequence of commands to download, configure, build, and install all dependencies:

1. Configure the dependencies CMake project with the desired installation path:
```console
cmake -S dependencies -B dependencies/build -DCMAKE_INSTALL_PREFIX=dependencies/installs
```
2. Download, build, and install all dependencies:
```console
cmake --build dependencies/build -j$(nproc)
```

## Building Mimir

Run the following sequence of commands to configure, build, and install Mimir:

1. Configure Mimir in the build directory `build/` with the `CMakePrefixPath` pointing to the installation directory of the dependencies:
```console
cmake -S . -B build -DCMAKE_PREFIX_PATH=${PWD}/dependencies/installs
```
2. Build Mimir in the build directory:
```console
cmake --build build -j$(nproc)
```
3. (Optional) Install Mimir from the build directory to the desired installation `prefix` directory:
```console
cmake --install build --prefix=<path/to/installation-directory>
```

The compiled executables should now be in either the build directory or the installation directory.
