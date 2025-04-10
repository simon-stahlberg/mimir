
#ifndef MIMIR_PYTHON_LANGUAGES_DESCRIPTION_LOGICS_BINDINGS_HPP
#define MIMIR_PYTHON_LANGUAGES_DESCRIPTION_LOGICS_BINDINGS_HPP

#include "../../init_declarations.hpp"

namespace mimir::bindings
{
template<languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
void bind_constructor(nb::module_& m, const std::string& class_name)
{
    nb::class_<languages::dl::IConstructor<D>>(m, class_name.c_str())
        .def("__str__",
             [](const languages::dl::IConstructor<D>& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
        .def("__repr__",
             [](const languages::dl::IConstructor<D>& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
        .def("evaluate", &languages::dl::IConstructor<D>::evaluate, nb::rv_policy::reference_internal, "evaluation_context"_a)
        .def("accept", &languages::dl::IConstructor<D>::accept, "visitor"_a)
        .def("get_index", &languages::dl::IConstructor<D>::get_index);

    nb::bind_vector<languages::dl::ConstructorList<D>>(m, (class_name + "List").c_str());
};
}

#endif