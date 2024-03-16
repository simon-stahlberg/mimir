#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

void init_formalism(py::module_&);
void init_search(py::module_&);

PYBIND11_MODULE(_pymimir, m)
{
    m.doc() = "Python bindings for the Mimir planning library.";

    py::module_ m_formalism = m.def_submodule("formalism", "The formalism submodule.");
    py::module_ m_search = m.def_submodule("search", "The search submodule.");

    init_formalism(m);
    init_search(m);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
