# Python Bindings

Current strategy:

- Introduce opague types where we would like to pass data back into c++
- There is no const in python. Hence, copy data from C++ to python where modifications would results in modifying internal memory.
We could avoid this by emulating const-correctness using a wrapper that restricts to data access.
- Use return policy reference_internal for simple non-owning raw pointers