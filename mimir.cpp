#include <Python.h>

static PyObject* mimir_say_hello(PyObject* self, PyObject* args) {
    const char* message = "Hello from mimir!";
    return Py_BuildValue("s", message);
}

static PyMethodDef mimir_methods[] = {
    {"SayHello", mimir_say_hello, METH_NOARGS, "Print a hello message."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mimir_module = {
    PyModuleDef_HEAD_INIT,
    "mimir",
    "A sample C++ module for Python",
    -1,
    mimir_methods
};

PyMODINIT_FUNC PyInit_mimir(void) {
    return PyModule_Create(&mimir_module);
}
