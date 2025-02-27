
#include "init_declarations.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

using namespace mimir;

namespace py = pybind11;

void init_formalism(py::module_& m) { py::class_<ActionImpl>(m, "Action"); }
