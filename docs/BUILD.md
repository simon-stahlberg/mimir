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

## Installing GCC 14.2 (c++23 support)

1. Installing GMP

```console
wget https://gcc.gnu.org/pub/gcc/infrastructure/gmp-6.2.1.tar.bz2
tar -xjf gmp-6.2.1.tar.bz2
mv gmp-6.2.1 gmp-6.2.1-src
export GMP_INSTALL_DIR=<path/to_installation>
cd gmp-6.2.1-src
./configure --prefix=$GMP_INSTALL_DIR
make -j$(nproc)
make install
cd ..
```

2. Installing MPFR

```console
wget https://gcc.gnu.org/pub/gcc/infrastructure/mpfr-4.1.0.tar.bz2
tar -xjf mpfr-4.1.0.tar.bz2
mv mpfr-4.1.0 mpfr-4.1.0-src
export MPFR_INSTALL_DIR=<path/to_installation>
cd mpfr-4.1.0-src
./configure --prefix=$MPFR_INSTALL_DIR --with-gmp=$GMP_INSTALL_DIR
make -j$(nproc)
make install
cd ..
```

3. Installing MPC

```console
wget https://gcc.gnu.org/pub/gcc/infrastructure/mpfr-1.2.1.tar.bz2
tar -xzf mpc-1.2.1.tar.gz
mv mpc-1.2.1 mpc-1.2.1-src
export MPC_INSTALL_DIR=<path/to_installation>
cd mpc-1.2.1-src
./configure --prefix=$MPC_INSTALL_DIR --with-gmp=$GMP_INSTALL_DIR --with-mpfr=$MPFR_INSTALL_DIR
make -j$(nproc)
make install
cd ..
```

4. Installing GCC

```console
git clone git@github.com:gcc-mirror/gcc.git gcc-14.2.0
git checkout release/gcc-14.2.0
cd gcc-14.2.0
export GCC_INSTALL_DIR=<path/to_installation>
./configure --prefix=$GCC_INSTALL_DIR --enable-languages=c,c++ --disable-multilib --with-gmp=$GMP_INSTALL_DIR --with-mpfr=$MPFR_INSTALL_DIR --with-mpc=$MPC_INSTALL_DIR
make -j$(nproc)
make install

export CC=${GCC_INSTALL_DIR}/bin/gcc
export CXX=${GCC_INSTALL_DIR}/bin/g++
export PATH=${GCC_INSTALL_DIR}/bin:$PATH
export LD_LIBRARY_PATH=${GCC_INSTALL_DIR}/lib64:$LD_LIBRARY_PATH
export LIBRARY_PATH=${GCC_INSTALL_DIR}/lib64:$LIBRARY_PATH
export CPATH=${GCC_INSTALL_DIR}/include:$CPATH
```

Alternatively using conda

```console
conda create --name gcc_env
conda activate gcc_env
conda install gxx=14.2.0 gcc=14.2.0 bzip2 xz zlib zstd
g++ --version
gcc --version
```