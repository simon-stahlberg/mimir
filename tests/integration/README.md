# Awesome-Planner 

Awesome-Planner is breadth-first search planner for lifted and grounded planning.

## Getting Started  

## Installing the Dependencies

Awesome-Planner depends on the following set of libraries:

- [Loki](https://github.com/drexlerd/Loki) for parsing PDDL files,
- [Boost](https://www.boost.org/) header-only libraries (Fusion, Spirit x3, Container, BGL),
- [Cista](https://github.com/felixguendling/cista/) for zero-copy serialization,
- [Mimir](https://github.com/simon-stahlberg/mimir) for planning functionality,
- [Nauty and Traces](https://users.cecs.anu.edu.au/~bdm/nauty/) for graph isomorphism testing, and
- [Abseil](https://github.com/abseil/abseil-cpp) for flat containers,

Run the following sequence of commands to download, configure, build, and install all dependencies:

1. Configure the dependencies CMake project with the desired installation path:
```console
cmake -S dependencies -B dependencies/build -DCMAKE_INSTALL_PREFIX=dependencies/installs -DCMAKE_PREFIX_PATH=$PWD/dependencies/installs
```
2. Download, build, and install all dependencies:
```console
cmake --build dependencies/build -j$(nproc)
```

## Building Awesome-Planner

Run the following sequence of commands to configure, build, and install Awesome-Planner:

1. Configure Awesome-Planner in the build directory `build/` with the `CMakePrefixPath` pointing to the installation directory of the dependencies:
```console
cmake -S . -B build -DCMAKE_PREFIX_PATH=${PWD}/dependencies/installs
```
2. Build Awesome-Planner in the build directory:
```console
cmake --build build -j$(nproc)
```
3. (Optional) Install Awesome-Planner from the build directory to the desired installation `prefix` directory:
```console
cmake --install build --prefix=<path/to/installation-directory>
```

The compiled executables should now be in either the build directory or the installation directory.
