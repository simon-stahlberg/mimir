# Mimir: Planner library

Mimir is a C++20 planning library for grounded and lifted planning. We created Mimir to be 1) efficient, 2) easy to integrate, use, and extend, and 3) used in multi-threaded machine learning applications. Mimir implements standard search algorithms such as breadth-first search and AStar search.

**Templated design:** The class interfaces are based on static polymorphism through the curiously recurring template pattern (CRTP) to enable heavy compiler optimizations.

**Zero heap-allocations:** Mimir implements a sub-library for serializing objects with fully customizable memory layouts and zero-cost de-serialization. Mimir reuses memory by moving buffers around in memory with zero heap allocations and deallocations.


## Getting Started

### Installing the Dependencies

Mimir depends on the PDDL parser [Loki](https://github.com/drexlerd/Loki), a fraction of [Boost's](boost.org) header-only libraries (Fusion, Spirit x3, Container), its performance benchmarking framework depends on [GoogleBenchmark](https://github.com/google/benchmark), and its testing framework depends on [GoogleTest](https://github.com/google/googletest).

We provide a CMake Superbuild project that takes care of downloading, building, and installing all dependencies.

```console
# Configure dependencies
cmake -S dependencies -B dependencies/build -DCMAKE_INSTALL_PREFIX=dependencies/installs
# Build and install dependencies
cmake --build dependencies/build -j16
```

### Building Mimir

```console
# Configure with installation prefixes of all dependencies
cmake -S . -B build -DCMAKE_PREFIX_PATH=${PWD}/dependencies/installs
# Build
cmake --build build -j16
# Install (optional)
cmake --install build --prefix=<path/to/installation-directory>
```

### Integrating Mimir

We provide a CMake Superbuild project [here](https://github.com/simon-stahlberg/mimir/tree/templated-design/tests/integration) that takes care of downloading, building, and installing Loki together and its dependencies. You can simply copy it to your project or integrate it in your own Superbuild and run it similarly to the Superbuild project from above.


## For Developers

### IDE Support

We developed Mimir in Visual Studio Code. We recommend installing the `C/C++` and `CMake Tools` extensions by Microsoft. To get maximum IDE support, you should set the following `Cmake: Configure Args` in the `CMake Tools` extension settings under `Workspace`:

- `-DCMAKE_PREFIX_PATH=${workspaceFolder}/dependencies/installs`

After running `CMake: Configure` in Visual Studio Code (ctrl + shift + p), you should see all include paths being correctly resolved.

### Argument passing

- Use prefix `ref_` for initialized output parameters and `out_` for non-initialized output parameters. Try to keep the number of output parameters as small as possible. Never use stack-allocated types as output parameters.

### Header includes

- Include only the most specialized headers to obtain the correct instantiation.
- Make use of inherited headers to keep the number of includes small.

### Tag Dispatching

- Use empty structs for tag dispatching. For example in a `SuccessorStateGenerator` we need to instantiate other types, derived on the state

```cpp
UnorderedSet<State<C>> m_states;
Builder<State<C>> m_state_builder;
```

This effectively instantiates corresponding types `View<State<C>>` and `Builder<State<C>>` depending on the configuration that is used.

### Concepts

- Use prefixes `Is` or `Has` to obtain more meaningful names.
- Apply concepts only in the requires part to make the distinction clearer

