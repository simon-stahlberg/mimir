
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

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_concept_atomic_state(nb::module_& m, const std::string& class_name)
{
    nb::class_<languages::dl::ConceptAtomicStateImpl<P>, languages::dl::IConstructor<languages::dl::ConceptTag>>(m, class_name.c_str())  //
        .def("get_predicate", &languages::dl::ConceptAtomicStateImpl<P>::get_predicate, nb::rv_policy::reference_internal);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_concept_atomic_goal(nb::module_& m, const std::string& class_name)
{
    nb::class_<languages::dl::ConceptAtomicGoalImpl<P>, languages::dl::IConstructor<languages::dl::ConceptTag>>(m, class_name.c_str())  //
        .def("get_predicate", &languages::dl::ConceptAtomicGoalImpl<P>::get_predicate, nb::rv_policy::reference_internal)
        .def("get_polarity", &languages::dl::ConceptAtomicGoalImpl<P>::get_polarity, nb::rv_policy::copy);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_role_atomic_state(nb::module_& m, const std::string& class_name)
{
    nb::class_<languages::dl::RoleAtomicStateImpl<P>, languages::dl::IConstructor<languages::dl::RoleTag>>(m, class_name.c_str())  //
        .def("get_predicate", &languages::dl::RoleAtomicStateImpl<P>::get_predicate, nb::rv_policy::reference_internal);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_role_atomic_goal(nb::module_& m, const std::string& class_name)
{
    nb::class_<languages::dl::RoleAtomicGoalImpl<P>, languages::dl::IConstructor<languages::dl::RoleTag>>(m, class_name.c_str())  //
        .def("get_predicate", &languages::dl::RoleAtomicGoalImpl<P>::get_predicate, nb::rv_policy::reference_internal)
        .def("get_polarity", &languages::dl::RoleAtomicGoalImpl<P>::get_polarity, nb::rv_policy::copy);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_boolean_atomic_state(nb::module_& m, const std::string& class_name)
{
    nb::class_<languages::dl::BooleanAtomicStateImpl<P>, languages::dl::IConstructor<languages::dl::BooleanTag>>(m, class_name.c_str())  //
        .def("get_predicate", &languages::dl::BooleanAtomicStateImpl<P>::get_predicate, nb::rv_policy::reference_internal);
};

template<languages::dl::IsConceptOrRoleTag D>
void bind_boolean_nonempty(nb::module_& m, const std::string& class_name)
{
    nb::class_<languages::dl::BooleanNonemptyImpl<D>, languages::dl::IConstructor<languages::dl::BooleanTag>>(m, class_name.c_str())
        .def("get_constructor", &languages::dl::BooleanNonemptyImpl<D>::get_constructor, nb::rv_policy::reference_internal);
};

template<languages::dl::IsConceptOrRoleTag D>
void bind_numerical_count(nb::module_& m, const std::string& class_name)
{
    nb::class_<languages::dl::NumericalCountImpl<D>, languages::dl::IConstructor<languages::dl::NumericalTag>>(m, class_name.c_str())
        .def("get_constructor", &languages::dl::NumericalCountImpl<D>::get_constructor, nb::rv_policy::reference_internal);
};
}

#endif