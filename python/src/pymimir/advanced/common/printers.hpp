
#ifndef MIMIR_PYTHON_COMMON_PRINTERS_HPP
#define MIMIR_PYTHON_COMMON_PRINTERS_HPP

#include <nanobind/nanobind.h>
#include <ostream>

namespace nb = nanobind;

namespace mimir
{
inline std::ostream& operator<<(std::ostream& os, const nb::tuple& tuple)
{
    os << nb::str(tuple).c_str();
    return os;
}
}

#endif