
#ifndef MIMIR_INIT_DECLARATIONS_HPP
#define MIMIR_INIT_DECLARATIONS_HPP

#include "mimir/mimir.hpp"
#include "mimir/common/types_cista.hpp"

#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors
#include <pybind11/stl_bind.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

//
// init - declarations:
//
void init_pymimir(pybind11::module_& m);

#endif  // MIMIR_INIT_DECLARATIONS_HPP
