# Mimir: A Generalized Planning Library

Mimir is a C++20-based generalized planning library with Python bindings, designed for both grounded and lifted planning. Its primary focus lies at the intersection of planning and machine learning, making it a powerful tool for research in learning-based planning. Unlike many planners, Mimir is designed to be used as a library rather than as a standalone application, offering seamless integration into both C++ and Python projects. Pre-compiled Python binaries are available via PyPI for easy installation and use.

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
  - `:numeric-fluents`
  - `:strips`
  - `:typing`
  - `:universal-preconditions`

- **Grounded and Lifted Planning:**
  Mimir supports the PDDL requirements for both grounded and lifted planning.
  In particular, lifted planning is critical for learning-based approaches because they often do not require access to grounded actions, resulting in significant speedups.

- **Collections of Problems:**
  Mimir supports parsing, translation, and in-memory manipulation for collections of problems.

- **Portability:**
  Setting up and running learning pipelines can be a challenging and time-consuming task.
  With this library, we aim to streamline this process by offering pre-compiled Python binaries, significantly reducing setup complexity.
  Additionally, Mimir simplifies the testing of trained models, allowing you to handle both training and evaluation without the need for an external planner.

- **Flexibility:**
  Although Mimir is tailored for learning-based planning, it also includes C++ implementations of search algorithms such as A*. The Python bindings allow the development of heuristics directly in Python. This facilitates rapid prototyping and the use of popular libraries such as NumPy. Mimir not only offers built-in implementations of search algorithms but also allows users to develop custom search algorithms directly in Python. Through its Python interface, users can access both grounded and lifted successor generators, as well as an implementation of a state repository.

- **Efficiency:**
  Mimir can also be included as a library in C++ projects, which is recommended for optimal performance. This library is optimized for lifted planning but our experiments show that the grounded mode is competitive with state-of-the-art planners. Our experimental evaluation is available [here](https://github.com/simon-stahlberg/mimir/tree/main/experiments/github).

## Technical Overview

- **PDDL Parser & Normalizer:**
  Mimir uses the [Loki](https://github.com/drexlerd/Loki) parser for handling PDDL files.
  Loki defines the grammar using the [Boost](https://www.boost.org/) library, enabling it to generate highly informative error messages for syntactically incorrect input. It also implements the normalization step of the input PDDL, mostly based on the method presented in section four of the paper [*"Concise finite-domain representations for PDDL planning tasks by Malte Helmert (AIJ 2009)"*](https://ai.dmi.unibas.ch/papers/helmert-aij2009.pdf).

- **Lifted Successor Generator:**
  The library implements a lifted applicable action generator based on the method presented in the paper [*"Lifted Successor Generation by Maximum Clique Enumeration"* by Simon Ståhlberg (ECAI 2023)](https://ebooks.iospress.nl/doi/10.3233/FAIA230516). This generator natively supports `:strips` and `:negative-preconditions`, while other precondition features are compiled away in the normalization step. We also extended the generator to natively support `:numeric-fluents` using partial function evaluation and standard interval arithmetics.

- **Grounded Successor Generator:**
  Delete relaxation is used to identify a set of grounded actions that may be applicable in potential successor states.
  This set is then used to initialize the data structure outlined in the article [*"The Fast Downward Planning System"* by Malte Helmert (JAIR 2006)](https://jair.org/index.php/jair/article/view/10457) to efficiently identify all applicable actions in given states. While this grounding process often results in significant performance improvements, it may not always be feasible in practice.

- **State Representation:**
  States are represented using a compressed sparse vector representation that contains all ground atoms that are true in the state. The compression step identifies the largest bit-width required to store all ground atoms. Internally, we unpack states into dense bitset representations for constant time randomized access.

- **Ground Action and Axiom Representations:**
  Ground action and axioms are complex composite structures. Similar to states, we represent them using a single compressed sparse vector. However, we do never unpack them, as having unpacked states suffices for efficient applicability checks.

## Getting Started (Python)

Mimir is available on [pypi](https://pypi.org/project/pymimir/).

```console
pip install pymimir
```

See [examples](docs/EXAMPLES_PYTHON.md) on how to use the Python library.

<!-- ## Getting Started (C++)

See [build](docs/BUILD.md) for how to compile Mimir, and [examples](docs/EXAMPLES_CPP.md) for how to use the C++ library. -->

## Getting Involved

We would love for people to participate in the development of Mimir, but we are not ready for that yet.
In the future, see [development guidelines](docs/DEVELOPER_GUIDELINES.md) for instructions.

## Long Term Goals

One of Mimir's long term goal is increasing the support of PDDL features towards non-deterministic, probabilistic, and numeric planning.
