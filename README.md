# Mimir: PDDL Parser and Planner Toolkit

Mimir is a C++ package designed to facilitate the parsing and inspection of Planning Domain Definition Language (PDDL) files. With Mimir, you can extract  information from PDDL files and navigate through the parsed representation using an object-oriented approach. The package also includes a lifted successor generator, allowing you to implement your own planners with ease. Additionally, Mimir provides the functionality to expand the entire state-space, enabling convenient generation of (labeled) datasets for machine learning tasks.

## Supported PDDL Subset

Mimir can handle STRIPS planning problems with negative preconditions and typing.


## Getting Started

### Dependencies

Loki depends on a fraction of [Boost's](boost.org) header-only libraries (Fusion, Spirit x3, Container), its performance benchmarking framework depends on [GoogleBenchmark](https://github.com/google/benchmark), and its testing framework depends on [GoogleTest](https://github.com/google/googletest).

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

- `-DCMAKE_PREFIX_PATH=<absolute/path/to/dependencies/installs`

After running `CMake: Configure` in Visual Studio Code (ctrl + shift + p), you should see all include paths being correctly resolved.


## Example Usage

To demonstrate the usage of the Mimir package, consider the following example in Python. This code snippet showcases the parsing of domain and problem files, using the lifted successor generator to generate all applicable actions for the initial state, and applying the last action to it.

```python
import argparse
import mimir

from pathlib import Path


def _parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('domain', type=Path, help='Path')
    parser.add_argument('problem', type=Path, help='Path')
    args = parser.parse_args()
    return args


def _main():
    args = _parse_arguments()
    domain_path: Path = args.domain
    problem_path: Path = args.problem
    domain = mimir.DomainParser(str(domain_path)).parse()
    problem = mimir.ProblemParser(str(problem_path)).parse(domain)
    successor_generator = mimir.LiftedSuccessorGenerator(problem)
    applicable_actions = successor_generator.get_applicable_actions(problem.initial)
    ground_action = applicable_actions[-1]
    successor_state = ground_action.apply(problem.initial)
    print(f'Initial state: {problem.initial.get_atoms()}')
    print(f'Ground action: {ground_action}')
    print(f'Successor state: {successor_state.get_atoms()}')
    print(f'Is applicable in successor state: {ground_action.is_applicable(successor_state)}')


if __name__ == '__main__':
    _main()
```

In this example, the program expects the paths to the domain and problem files as command-line arguments. When executing the program with `python3 plan.py <PATH>/domain.pddl <PATH>/gripper_b-1.pddl`, the following output is generated:

```
Initial state: [<Atom 'room(rooma)'>, <Atom 'room(roomb)'>, <Atom 'ball(ball1)'>, <Atom 'gripper(left)'>, <Atom 'gripper(right)'>, <Atom 'at-robby(rooma)'>, <Atom 'at(ball1, rooma)'>, <Atom 'free(left)'>, <Atom 'free(right)'>]
Ground action: <Action 'pick(ball1, rooma, left)'>
Successor state: [<Atom 'room(rooma)'>, <Atom 'room(roomb)'>, <Atom 'ball(ball1)'>, <Atom 'gripper(left)'>, <Atom 'gripper(right)'>, <Atom 'at-robby(rooma)'>, <Atom 'free(right)'>, <Atom 'carry(ball1, left)'>]
Is applicable in successor state: False
```
