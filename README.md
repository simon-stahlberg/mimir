# Mimir: Planner library

Mimir is a C++20 planning library for grounded and lifted planning. We created Mimir to be 1) efficient, 2) easy to integrate, use, and extend. Mimir implements standard search algorithms such as breadth-first search and AStar search. Mimir also provides Python bindings for the most common template instantiations.

**Flatmemory:** Mimir serializes data types using [flatmemory](https://github.com/drexlerd/flatmemory) into cache-efficient flat memory layouts with zero-cost de-serialization. Mimir uses it for states, search nodes, grounded actions, and grounded axioms.

## Example C++ API

```cpp
const auto parser = PDDLParser("domain.pddl", "problem.pddl")
const auto aag = std::make_shared<LiftedAAG>(parser.get_problem(), parser.get_factories())
const auto brfs = BrFsAlgorithm(aag)
const auto [status, plan] = brfs.find_solution()
```

## Example Python API

```python
parser = PDDLParser("domain.pddl", "problem.pddl")
aag = LiftedAAG(parser.get_problem(), parser.get_factories())
brfs = BrFsAlgorithm(aag)
status, plan = brfs.find_solution()
```

## Getting Started

### Installing the Dependencies

Mimir depends on the PDDL parser [Loki](https://github.com/drexlerd/Loki), a fraction of [Boost's](boost.org) header-only libraries (Fusion, Spirit x3, Container), memory layout flattening on [flatmemory](https://github.com/drexlerd/flatmemory), its performance benchmarking framework depends on [GoogleBenchmark](https://github.com/google/benchmark), and its testing framework depends on [GoogleTest](https://github.com/google/googletest).

We provide a CMake Superbuild project that takes care of downloading, building, and installing all dependencies.

```console
# Configure dependencies
cmake -S dependencies -B dependencies/build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=dependencies/installs
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

### Creating your own Planner based on Mimir

We provide a CMake Superbuild project [here](https://github.com/simon-stahlberg/mimir/tree/dynamic/tests/integration) that takes care of downloading, building, and installing Loki together and its dependencies. You can simply copy it to your project or integrate it in your own Superbuild and run it similarly to the Superbuild project from above.


## For Developers

### IDE Support

We developed Mimir in Visual Studio Code. We recommend installing the `C/C++` and `CMake Tools` extensions by Microsoft. To get maximum IDE support, you should set the following `Cmake: Configure Args` in the `CMake Tools` extension settings under `Workspace`:

- `-DCMAKE_PREFIX_PATH=${workspaceFolder}/dependencies/installs`

After running `CMake: Configure` in Visual Studio Code (ctrl + shift + p), you should see all include paths being correctly resolved.

Alternatively, you can create the file `.vscode/settings.json` with the content:

```json
{
    "cmake.configureArgs": [ "-DCMAKE_PREFIX_PATH=${workspaceFolder}/dependencies/installs" ]
}
```

### Argument passing

- Use prefix `ref_` for initialized output parameters and `out_` for non-initialized output parameters. Try to keep the number of output parameters as small as possible. Never use stack-allocated types as output parameters.

### Concepts

- Use prefixes `Is` or `Has` to obtain more meaningful names.
