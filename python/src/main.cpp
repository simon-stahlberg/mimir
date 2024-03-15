#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

void init_search(py::module_&);

PYBIND11_MODULE(_mimir, m)
{
    m.doc() = "Python bindings for the dlplan description logics first-order features for planning";

    py::module_ m_search = m.def_submodule("search", "The core submodule.");

    init_search(m);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
