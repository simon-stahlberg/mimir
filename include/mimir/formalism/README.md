# formalism subdirectory

This subdirectory contains the pddl data structure.

Each PDDL type of mimir contains currently an automatic memory layouting done by the compiler. In the future, we might want to implement `Builders` following the pattern of [flatbuffers](https://flatbuffers.dev/) where we make serialize into a `std::vector<char>` that contains a header with meta data where we can find specific data and use `reinterpret_cast` to work on the actual data type.


# Naming Conventions

- Differentiate types that have a grounded from a partially grounded version with a prefix `Ground`. For example, `GroundAtom` and `Atom`. Notice that although a `Problem` is ground there is no prefix because there does not exist partially grounded problems.
