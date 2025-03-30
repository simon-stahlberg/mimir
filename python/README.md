# Python Bindings Design Strategy

For efficiency and correctness reasons we follow two design principles:

1. For every C++ type, there must be corresponding Python type, e.g., C++ `StateList` <=> Python `StateList`. We specifically disallow type casters because they are an ad-hoc solution that strictly require copying.
2. For every mutable C++ type `T`, there must be two corresponding Python types `T` and `ImmutableT`. This should be done by providing a binding for `PyImmutable<T>` with name `ImmutableT`. The binding for `PyImmutable<T>` should only contain access to const methods. Now, when returning a const reference to an mutable type `T`, we can simply wrap it into `PyImmutable<T>` to restrict the python user interface toward immutability. An alternative way is simply copying such that mutation does not modify internal data but it is much less efficient.

# Open tasks

- Create template machinary needed to emulate const correctness using `PyImmutable<T>`
