#include <pybind11/pybind11.h>
#include <Python.h>

int add(int a, int b) { return a + b; }

PYBIND11_MODULE(mimir, m) { m.def("add", &add, "A function that adds two numbers"); }
