# formalism subdirectory

This subdirectory contains the PDDL data structure.

Each PDDL type of mimir has automatic memory layouts done by the compiler. In the future, we might want to implement `Builders` following the pattern of [flatbuffers](https://flatbuffers.dev/) where we serialize into a `std::vector<char>` that contains a header with meta-data where we can find specific data and use `reinterpret_cast` to work on the actual data type. The current API where we return pointers will work nicely with this design pattern.


# Naming Conventions

- Differentiate types that have a grounded from a partially grounded version with the prefix `Ground`. For example, `GroundAtom` and `Atom`. Notice that although a `Problem` is ground there is no prefix because there does not exist partially grounded problems.
