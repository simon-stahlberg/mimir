
#ifndef MIMIR_PYTHON_LANGUAGES_GENERAL_POLICIES_BINDINGS_HPP
#define MIMIR_PYTHON_LANGUAGES_GENERAL_POLICIES_BINDINGS_HPP

#include "../../init_declarations.hpp"

namespace mimir::languages::general_policies
{
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
void bind_named_feature(nb::module_& m, const std::string& class_name)
{
    using Type = NamedFeatureImpl<D>;

    nb::class_<Type>(m, class_name.c_str())
        .def("__str__", [](const Type& self) { return to_string(self); })
        .def("__eq__", [](const Type& lhs, const Type& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const Type& lhs, const Type& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const Type& self) { return &self; })
        .def("get_index", &Type::get_index)
        .def("get_name", &Type::get_name, nb::rv_policy::reference_internal)
        .def("get_feature", &Type::get_feature, nb::rv_policy::reference_internal);

    nb::bind_vector<NamedFeatureList<D>>(m, (class_name + "List").c_str());
};
}

#endif