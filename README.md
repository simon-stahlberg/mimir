# Mimir: Planner library

Mimir is a C++20 planning library with Python bindings for grounded and lifted classical planning.

*Focus on the intersection of planning and learning - search algorithms, parallel data processing, and knowledge representation languages.*

- Supports a rich fragment of PDDL in both the grounded and lifted planning setup -
strips, typing, negative-preconditions, disjunctive-preconditions, equality,
existential-preconditions, universal-preconditions, conditional-effects, derived-predicates, action-costs.
- Fully written in C++ - no intermediate files.
- Provides Python bindings that even allow you to write heuristics for AStar search directly in Python.
- Provides a generic graph library that supports forward and backward traversal of edges with adapters to use the powerful Boost BGL library.
- Uses the zero-copy serialization library Cista for efficient parallel data generation.

## 2. Getting Started (Python)

Mimir is available on [pypi](https://pypi.org/project/pymimir/).

```console
pip install pymimir
```

### 2.1. Example Python API

```python
import pymimir as mm

parser = mm.PDDLParser("domain.pddl", "problem.pddl")
aag = mm.LiftedApplicableActionGenerator(parser.get_problem(), parser.get_pddl_factories())
brfs = mm.BrFSAlgorithm(aag)
status, plan = brfs.find_solution()
```

## 3. Getting Started (C++)

### 3.2. Installing the Dependencies

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

### 3.3. Building Mimir

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

### 3.1. Example C++ API

```cpp
#include  <mimir/mimir.hpp>

const auto parser = PDDLParser("domain.pddl", "problem.pddl")
const auto aag = std::make_shared<LiftedApplicableActionGenerator>(parser.get_problem(), parser.get_pddl_factories())
const auto brfs = BrFSAlgorithm(aag)
const auto [status, plan] = brfs.find_solution()
```

## 4.For Developers

### 4.1. IDE Support

We developed Mimir in Visual Studio Code. We recommend installing the `C/C++` and `CMake Tools` extensions by Microsoft. To get maximum IDE support, you should set the following `Cmake: Configure Args` in the `CMake Tools` extension settings under `Workspace`:

-  `-DCMAKE_PREFIX_PATH=${workspaceFolder}/dependencies/installs`

After running `CMake: Configure` in Visual Studio Code (ctrl + shift + p), you should see all include paths being correctly resolved.

Alternatively, you can create the file `.vscode/settings.json` with the content:

```json
{
"cmake.configureArgs": [ "-DCMAKE_PREFIX_PATH=${workspaceFolder}/dependencies/installs" ]
}
```

