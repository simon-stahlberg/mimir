#include "../init_declarations.hpp"

using namespace mimir;

void bind_common(nb::module_& m)
{
    nb::class_<FlatDoubleList>(m, "FlatDoubleList");

    nb::class_<FlatIndexList>(m, "FlatIndexList");

    nb::class_<FlatBitset>(m, "FlatBitset");
}