# Mimir: Planner library

Mimir is a C++20 planning library with Python bindings for grounded and lifted planning. We created Mimir to be 1) efficient, 2) easy to integrate, use, and extend. Mimir implements standard search algorithms such as breadth-first search and AStar search.

## Supported PDDL Requirements

Mimir supports the following PDDL requirements in the grounded and lifting setting.

- [x] :strips
- [x] :typing
- [x] :negative-preconditions
- [x] :disjunctive-preconditions
- [x] :equality
- [x] :existential-preconditions
- [x] :universal-preconditions
- [x] :quantified-preconditions
- [x] :conditional-effects
- [x] :adl
- [x] :derived-predicates
- [x] :action-costs

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

Mimir depends on the following set of libraries:

- [Loki](https://github.com/drexlerd/Loki) for parsing PDDL files,
- [Boost](boost.org) header-only libraries (Fusion, Spirit x3, Container),
- [flatmemory](https://github.com/drexlerd/flatmemory) for flattening memory layouts of complex composite types,
- [nauty and Traces](https://pallini.di.uniroma1.it/) for isomorphism testing,
- [GoogleBenchmark](https://github.com/google/benchmark) for automated performance benchmarking, and
- [GoogleTest](https://github.com/google/googletest) for unit testing.

Run the following sequence of commands to commands to download, configure, build, and install all dependencies:

```console
# Configure dependencies
cmake -S dependencies -B dependencies/build -DCMAKE_INSTALL_PREFIX=dependencies/installs
# Build and install dependencies
cmake --build dependencies/build -j16
```

### Building Mimir

Run the following sequence of commands to configure, build, and install Mimir:

```console
# Configure with installation prefixes of all dependencies
cmake -S . -B build -DCMAKE_PREFIX_PATH=${PWD}/dependencies/installs
# Build
cmake --build build -j16
# Install (optional)
cmake --install build --prefix=<path/to/installation-directory>
```

### Creating your own Planner based on Mimir

We provide a CMake Superbuild project [here](https://github.com/simon-stahlberg/mimir/tree/dynamic/tests/integration) that downloads, builds, and installs Mimir and all its dependencies. We recommend using it as a dependency project for your project, similar to how we handle Mimir's dependencies.

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
