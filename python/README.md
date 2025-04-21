# Python Bindings Design Strategy

For efficiency and correctness reasons, we follow two design principles:

1. One-to-one-mapping: 

Every C++ type must have a corresponding Python type, e.g., C++ `StateList` <=> Python `StateList`. We discourage type casters because they strictly copy back and forth.

2. Emulating const-semantics: 

Every C++ type `T` with functions that mutate the object must have two corresponding Python types, `T` and `ImmutableT`. This should be done by providing a bindings for `T` with the name `T` and `PyImmutable<T>` with the name `ImmutableT`. The binding of `T` contains all functions and the binding for `PyImmutable<T>` must only contain const methods. Therefore, `PyImmutable<T>` restricts the interface and we can simply wrap return values when returning `const T&`. 

# Open Issues/Tasks

- Create template machinery needed to emulate const correctness using `PyImmutable<T>`.
- Free functions such as `graphs::get_state(const graphs::ProblemVertex& vertex)` defined in datasets results in cast errors, i.e, "Unable to convert function return value to a Python type!". A solution is returning py::object but that results in unsatisfying generic stubs.
