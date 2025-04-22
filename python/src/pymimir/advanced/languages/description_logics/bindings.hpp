
#ifndef MIMIR_PYTHON_LANGUAGES_DESCRIPTION_LOGICS_BINDINGS_HPP
#define MIMIR_PYTHON_LANGUAGES_DESCRIPTION_LOGICS_BINDINGS_HPP

#include "../../init_declarations.hpp"

namespace mimir::languages::dl
{
/**
 * DL
 */

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void bind_constructor(nb::module_& m, const std::string& class_name)
{
    using ValueType = IConstructor<D>;

    nb::class_<ValueType>(m, class_name.c_str())
        .def("__str__", [](const ValueType& self) { return to_string(self); })
        .def("__eq__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const ValueType& self) { return std::hash<const ValueType*> {}(&self); })
        .def("evaluate", &ValueType::evaluate, nb::rv_policy::reference_internal, "evaluation_context"_a)
        .def("accept", &ValueType::accept, "visitor"_a)
        .def("get_index", &ValueType::get_index);

    nb::bind_vector<ConstructorList<D>>(m, (class_name + "List").c_str());
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_concept_atomic_state(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<ConceptTag>;
    using ValueType = ConceptAtomicStateImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_concept_atomic_goal(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<ConceptTag>;
    using ValueType = ConceptAtomicGoalImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal)
        .def("get_polarity", &ValueType::get_polarity, nb::rv_policy::copy);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_role_atomic_state(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<RoleTag>;
    using ValueType = RoleAtomicStateImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_role_atomic_goal(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<RoleTag>;
    using ValueType = RoleAtomicGoalImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal)
        .def("get_polarity", &ValueType::get_polarity, nb::rv_policy::copy);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_boolean_atomic_state(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<BooleanTag>;
    using ValueType = BooleanAtomicStateImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal);
};

template<IsConceptOrRoleTag D>
void bind_boolean_nonempty(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<BooleanTag>;
    using ValueType = BooleanNonemptyImpl<D>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_constructor", &ValueType::get_constructor, nb::rv_policy::reference_internal);
};

template<IsConceptOrRoleTag D>
void bind_numerical_count(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<NumericalTag>;
    using ValueType = NumericalCountImpl<D>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_constructor", &ValueType::get_constructor, nb::rv_policy::reference_internal);
};

/**
 * CNFGrammar
 */

namespace cnf_grammar
{
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void bind_constructor(nb::module_& m, const std::string& class_name)
{
    using ValueType = IConstructor<D>;

    nb::class_<ValueType>(m, class_name.c_str())
        .def("__str__", [](const ValueType& self) { return to_string(self); })
        .def("__eq__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const ValueType& self) { return std::hash<const ValueType*> {}(&self); })
        .def("test_match", &ValueType::test_match, nb::rv_policy::copy, "constructor"_a, "grammar"_a)
        .def("accept", &ValueType::accept, "visitor"_a)
        .def("get_index", &ValueType::get_index);

    nb::bind_vector<ConstructorList<D>>(m, (class_name + "List").c_str());
};

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void bind_nonterminal(nb::module_& m, const std::string& class_name)
{
    using ValueType = NonTerminalImpl<D>;

    nb::class_<ValueType>(m, class_name.c_str())
        .def("__str__", [](const ValueType& self) { return to_string(self); })
        .def("__eq__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const ValueType& self) { return std::hash<const ValueType*> {}(&self); })
        .def("test_match", &ValueType::test_match, nb::rv_policy::copy, "constructor"_a, "grammar"_a)
        .def("accept", &ValueType::accept, "visitor"_a)
        .def("get_index", &ValueType::get_index)
        .def("get_name", &ValueType::get_name, nb::rv_policy::copy);

    nb::bind_vector<NonTerminalList<D>>(m, (class_name + "List").c_str());
};

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void bind_derivation_rule(nb::module_& m, const std::string& class_name)
{
    using ValueType = DerivationRuleImpl<D>;

    nb::class_<ValueType>(m, class_name.c_str())
        .def("__str__", [](const ValueType& self) { return to_string(self); })
        .def("__eq__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const ValueType& self) { return std::hash<const ValueType*> {}(&self); })
        .def("test_match", &ValueType::test_match, nb::rv_policy::copy, "constructor"_a, "grammar"_a)
        .def("accept", &ValueType::accept, "visitor"_a)
        .def("get_index", &ValueType::get_index)
        .def("get_head", &ValueType::get_head, nb::rv_policy::reference_internal)
        .def("get_body", &ValueType::get_body, nb::rv_policy::reference_internal);

    nb::bind_vector<DerivationRuleList<D>>(m, (class_name + "List").c_str());
};

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void bind_substitution_rule(nb::module_& m, const std::string& class_name)
{
    using ValueType = SubstitutionRuleImpl<D>;

    nb::class_<ValueType>(m, class_name.c_str())
        .def("__str__", [](const ValueType& self) { return to_string(self); })
        .def("__eq__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const ValueType& self) { return std::hash<const ValueType*> {}(&self); })
        .def("test_match", &ValueType::test_match, nb::rv_policy::copy, "constructor"_a, "grammar"_a)
        .def("accept", &ValueType::accept, "visitor"_a)
        .def("get_index", &ValueType::get_index)
        .def("get_head", &ValueType::get_head, nb::rv_policy::reference_internal)
        .def("get_body", &ValueType::get_body, nb::rv_policy::reference_internal);

    nb::bind_vector<SubstitutionRuleList<D>>(m, (class_name + "List").c_str());
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_concept_atomic_state(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<ConceptTag>;
    using ValueType = ConceptAtomicStateImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_concept_atomic_goal(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<ConceptTag>;
    using ValueType = ConceptAtomicGoalImpl<P>;

    nb::class_<ValueType, BaseType>(m,
                                    class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal)
        .def("get_polarity", &ValueType::get_polarity, nb::rv_policy::copy);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_role_atomic_state(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<RoleTag>;
    using ValueType = RoleAtomicStateImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_role_atomic_goal(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<RoleTag>;
    using ValueType = RoleAtomicGoalImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal)
        .def("get_polarity", &ValueType::get_polarity, nb::rv_policy::copy);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_boolean_atomic_state(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<BooleanTag>;
    using ValueType = BooleanAtomicStateImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal);
};

template<IsConceptOrRoleTag D>
void bind_boolean_nonempty(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<BooleanTag>;
    using ValueType = BooleanNonemptyImpl<D>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_constructor", &ValueType::get_nonterminal, nb::rv_policy::reference_internal);
};

template<IsConceptOrRoleTag D>
void bind_numerical_count(nb::module_& m, const std::string& class_name)
{
    using BaseType = IConstructor<NumericalTag>;
    using ValueType = NumericalCountImpl<D>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_constructor", &ValueType::get_nonterminal, nb::rv_policy::reference_internal);
};
}
}

#endif