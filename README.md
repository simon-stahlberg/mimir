# Mimir: Planner library

Mimir is a C++20 planning library with Python bindings for grounded and lifted planning. We created Mimir to be 1) efficient, 2) easy to integrate, use, and extend. Mimir implements standard search algorithms such as breadth-first search and AStar search.

## 1. Supported PDDL Requirements

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

## 2. Getting Started (Python)

### 2.1. Example Python API

```python
parser = PDDLParser("domain.pddl", "problem.pddl")
aag = LiftedAAG(parser.get_problem(), parser.get_factories())
brfs = BrFSAlgorithm(aag)
status, plan = brfs.find_solution()
```

### 2.2. Installing the Python Bindings Pymimir

Mimir is available on [pypi](https://pypi.org/project/pymimir/).

```console
pip install pymimir
```

### 2.3. Ownership Semantics

***Important for Python Users***

For simplicity and efficiency reasons, the Python bindings use the same ownership semantics as its underlying C++ code.
The ownership semantics are very simple and it is extremely important to get them correct for avoiding segfaults.
We provide an [example](https://github.com/simon-stahlberg/mimir/tree/dynamic/python/examples/ownership_semantics.py) that illustrates three rules for correct ownership semantics.

## 3. Getting Started (C++)

### 3.1. Example C++ API

```cpp
const auto parser = PDDLParser("domain.pddl", "problem.pddl")
const auto aag = std::make_shared<LiftedAAG>(parser.get_problem(), parser.get_factories())
const auto brfs = BrFSAlgorithm(aag)
const auto [status, plan] = brfs.find_solution()
```

### 3.2. Installing the Dependencies

Mimir depends on the following set of libraries:

- [Loki](https://github.com/drexlerd/Loki) for parsing PDDL files,
- [Boost](https://www.boost.org/) header-only libraries (Fusion, Spirit x3, Container),
- [Flatmemory](https://github.com/drexlerd/flatmemory) for flattening memory layouts of complex composite types,
- [Nauty and Traces](https://users.cecs.anu.edu.au/~bdm/nauty/) for graph isomorphism testing,
- [GoogleBenchmark](https://github.com/google/benchmark) for automated performance benchmarking, and
- [GoogleTest](https://github.com/google/googletest) for unit testing.

Run the following sequence of commands to download, configure, build, and install all dependencies:

```console
# Configure dependencies
cmake -S dependencies -B dependencies/build -DCMAKE_INSTALL_PREFIX=dependencies/installs
# Build and install dependencies
cmake --build dependencies/build -j16
```

### 3.3. Building Mimir

Run the following sequence of commands to configure, build, and install Mimir:

```console
# Configure with installation prefixes of all dependencies
cmake -S . -B build -DCMAKE_PREFIX_PATH=${PWD}/dependencies/installs
# Build
cmake --build build -j16
# Install (optional)
cmake --install build --prefix=<path/to/installation-directory>
```

### 3.4. Creating your own Planner based on Mimir

We provide a CMake Superbuild project [here](https://github.com/simon-stahlberg/mimir/tree/dynamic/tests/integration) that downloads, builds, and installs Mimir and all its dependencies. We recommend using it as a dependency project for your project, similar to how we handle Mimir's dependencies. Furthermore, to ensure that your dependencies keep working even after changes to the dependencies repos, we recommend explicitly setting the `GIT_TAG` of each dependency to the commit hash that you started developing on.

## 4.For Developers

### 4.1. IDE Support

We developed Mimir in Visual Studio Code. We recommend installing the `C/C++` and `CMake Tools` extensions by Microsoft. To get maximum IDE support, you should set the following `Cmake: Configure Args` in the `CMake Tools` extension settings under `Workspace`:

- `-DCMAKE_PREFIX_PATH=${workspaceFolder}/dependencies/installs`

After running `CMake: Configure` in Visual Studio Code (ctrl + shift + p), you should see all include paths being correctly resolved.

Alternatively, you can create the file `.vscode/settings.json` with the content:

```json
{
    "cmake.configureArgs": [ "-DCMAKE_PREFIX_PATH=${workspaceFolder}/dependencies/installs" ]
}
```

### 4.2. Argument passing

- Use prefix `ref_` for initialized output parameters and `out_` for non-initialized output parameters. Try to keep the number of output parameters as small as possible. Never use stack-allocated types as output parameters.

### 4.3. Concepts

- Every template parameter must be constrained using a suitable concept.

### 4.4. Memory allocations

- Reuse preallocated memory where possible, leave copying up to the user by returning const references to internal memory. For example, the `ObjectGraphFactory` returns a const reference to an `ObjectGraph`. The user can decide to copy it for persistent storage, or perform temporary operations such as computing a graph certificate.

### 4.5. Constructors

- Arguments passed to the constructor should be passed by value to avoid having to write duplicate constructors. Users have the responsibility to move arguments into the constructors. Exceptions to the rules are types that cannot be copied, e.g., `std::unique_ptr<T>`

### 4.6. Keyword "auto"

- Use the keyword `auto` as much as possible to avoid automatic conversations and flexibility.
- Use left-to-right notation for readability, e.g., `auto vec = std::vector<int>{};`

### 4.7. Interfaces

- Use pure virtual interfaces if a type is only used through a pointer to the interface.
- Use more expressive external inheritance hierarchies through type erasure if a type is used either through its concrete type or its abstract type.
