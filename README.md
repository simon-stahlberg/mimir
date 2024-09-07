# Mimir: A Planning Library

Mimir is a C++20-based planning library with Python bindings, designed for both grounded and lifted planning.
Its primary focus lies at the intersection of planning and machine learning, making it a powerful tool for research in learning-based planning.
Unlike many planners, Mimir is designed to be used as a library rather than as a standalone application, offering seamless integration into both C++ and Python projects.
Pre-compiled Python binaries are available via PyPI for easy installation and use.

## Key Features

- **Rich PDDL Support:**
  Mimir meets a wide range of Planning Domain Definition Language (PDDL) requirements:
  - `:action-costs`
  - `:conditional-effects`
  - `:derived-predicates`
  - `:disjunctive-preconditions`
  - `:equality`
  - `:existential-preconditions`
  - `:negative-preconditions`
  - `:strips`
  - `:typing`
  - `:universal-preconditions`

- **Grounded and Lifted Planning:**
  Mimir supports the PDDL requirements for both grounded and lifted planning.
  In particular, lifted planning is critical for learning-based approaches because they often do not require access to grounded actions, resulting in significant speedups.

- **Portability:**
  Setting up and running learning pipelines can be a challenging and time-consuming task.
  With this library, we aim to streamline this process by offering pre-compiled Python binaries, significantly reducing setup complexity.
  Additionally, Mimir simplifies the testing of trained models, allowing you to handle both training and evaluation without the need for an external planner.

- **Flexibility:**
  Although Mimir is tailored for learning-based planning, it also includes C++ implementations of search algorithms such as A*.
  The Python bindings allow the development of heuristics directly in Python.
  This facilitates rapid prototyping and the use of popular libraries such as NumPy.
  Mimir not only offers built-in implementations of search algorithms but also allows users to develop custom search algorithms directly in Python.
  Through its Python interface, users can access both grounded and lifted successor generators, as well as an implementation of a state repository.

- **Efficiency:**
  Mimir can also be included as a library in C++ projects, which is recommended for optimal performance.
  While this library is optimized, it is designed with readability and ease of use in mind, some planners may outperform it in terms of raw state expansion speed and memory usage.

## Getting Started (Python)

Mimir is available on [pypi](https://pypi.org/project/pymimir/).

```console
pip install pymimir
```

See [examples](docs/EXAMPLES_PYTHON.md) on how to use the Python library.

## Getting Started (C++)

See [build](docs/BUILD.md) for how to compile Mimir, and [examples](docs/EXAMPLES_PYTHON.md) for how to use the C++ library.

## Getting Involved

We would love for people to participate in the development of Mimir, but we are not ready for that yet.
In the future, see [development guidelines](docs/DEVELOPER_GUIDELINES.md) for instructions.
