#include "../init_declarations.hpp"

using namespace mimir;

namespace mimir::common
{

void bind_module_definitions(nb::module_& m)
{
    nb::class_<FlatDoubleList>(m, "FlatDoubleList").def(nb::init<>());

    nb::class_<FlatIndexList>(m, "FlatIndexList").def(nb::init<>());

    nb::class_<FlatBitset>(m, "FlatBitset").def(nb::init<>());
}

}