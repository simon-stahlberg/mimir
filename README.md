# Mimir: Planner library

Mimir is a C++20 planning library for grounded and lifted planning. We created Mimir to be 1) efficient, 2) highly flexible, 3) easy to integrate, use, and extend, and 4) used in multi-threaded machine learning applications. Mimir implements standard search algorithms such as breadth-first search and AStar search.

**Static dispatch:** Mimir is fully based on static dispatch. The planner configuration is given at compile time allowing the compiler to produce faster code. User's are also to make decisions at compile time using template meta-programming techniques based on their planner configuration to produce even faster code.

**Zero heap-allocations:** Mimir implements a sub-library for serializing objects with fully customizable memory layouts and zero-cost de-serialization. Mimir reuses memory by moving buffers around in memory with zero heap allocations and deallocations.

## Example API

```cpp
planner = Planner<AStarTag<LiftedTag, BlindTag>>("domain.pddl", "problem.pddl")
// auto plan = TODO: add simple way to get the type of a plan.
const auto status = planner.find_solution(plan)
```


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

### Directory Structure

- Put base classes in a header file with the suffix `template.hpp` in subdirectory `<name>s`.
- Put declarations of a derived class with tag `<tag>` in the subdirectory `<name>s`.
- Put a specialization of the derived class in a header file with the name `<tag>-<tags...>.hpp` in the subdirectory `<name>s` where `<tags...>` is a list of `_` separated tags that identify the specialization.
- Include all specializations of the derived class with tag `<tag>` in `<tag>s.hpp`.

We sometimes use reasonable shortcuts for `<tag>` to make it more readable.

### Header includes

- Include the header that contains all specializations of a derived class with tag `<tag>`.

### Concepts

- Use prefixes `Is` or `Has` to obtain more meaningful names.
- Always apply `Is` concepts in the template argument declaration to provide type hints for the IDE.

