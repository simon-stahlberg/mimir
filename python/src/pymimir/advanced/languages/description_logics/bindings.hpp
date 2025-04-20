
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
        .def("__eq__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs == &rhs; })
        .def("__hash__", [](const ValueType& self) { return std::hash<const ValueType*> {}(&self); })
        .def("__str__",
             [](const ValueType& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
        .def("__repr__",
             [](const ValueType& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
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

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void bind_cnf_grammar_constructor(nb::module_& m, const std::string& class_name)
{
    using ValueType = cnf_grammar::IConstructor<D>;

    nb::class_<ValueType>(m, class_name.c_str())
        .def("__eq__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs == &rhs; })
        .def("__hash__", [](const ValueType& self) { return std::hash<const ValueType*> {}(&self); })
        .def("__str__",
             [](const ValueType& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
        .def("__repr__",
             [](const ValueType& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
        .def("test_match", &ValueType::test_match, nb::rv_policy::copy, "constructor"_a, "grammar"_a)
        .def("accept", &ValueType::accept, "visitor"_a)
        .def("get_index", &ValueType::get_index);

    nb::bind_vector<cnf_grammar::ConstructorList<D>>(m, (class_name + "List").c_str());
};

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void bind_cnf_grammar_nonterminal(nb::module_& m, const std::string& class_name)
{
    using ValueType = cnf_grammar::NonTerminalImpl<D>;

    nb::class_<ValueType>(m, class_name.c_str())
        .def("__eq__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs == &rhs; })
        .def("__hash__", [](const ValueType& self) { return std::hash<const ValueType*> {}(&self); })
        .def("__str__",
             [](const ValueType& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
        .def("__repr__",
             [](const ValueType& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
        .def("test_match", &ValueType::test_match, nb::rv_policy::copy, "constructor"_a, "grammar"_a)
        .def("accept", &ValueType::accept, "visitor"_a)
        .def("get_index", &ValueType::get_index)
        .def("get_name", &ValueType::get_name, nb::rv_policy::copy);

    nb::bind_vector<cnf_grammar::NonTerminalList<D>>(m, (class_name + "List").c_str());
};

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void bind_cnf_grammar_derivation_rule(nb::module_& m, const std::string& class_name)
{
    using ValueType = cnf_grammar::DerivationRuleImpl<D>;

    nb::class_<ValueType>(m, class_name.c_str())
        .def("__eq__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs == &rhs; })
        .def("__hash__", [](const ValueType& self) { return std::hash<const ValueType*> {}(&self); })
        .def("__str__",
             [](const ValueType& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
        .def("__repr__",
             [](const ValueType& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
        .def("test_match", &ValueType::test_match, nb::rv_policy::copy, "constructor"_a, "grammar"_a)
        .def("accept", &ValueType::accept, "visitor"_a)
        .def("get_index", &ValueType::get_index)
        .def("get_head", &ValueType::get_head, nb::rv_policy::reference_internal)
        .def("get_body", &ValueType::get_body, nb::rv_policy::reference_internal);

    nb::bind_vector<cnf_grammar::DerivationRuleList<D>>(m, (class_name + "List").c_str());
};

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void bind_cnf_grammar_substitution_rule(nb::module_& m, const std::string& class_name)
{
    using ValueType = cnf_grammar::SubstitutionRuleImpl<D>;

    nb::class_<ValueType>(m, class_name.c_str())
        .def("__eq__", [](const ValueType& lhs, const ValueType& rhs) { return &lhs == &rhs; })
        .def("__hash__", [](const ValueType& self) { return std::hash<const ValueType*> {}(&self); })
        .def("__str__",
             [](const ValueType& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
        .def("__repr__",
             [](const ValueType& self)
             {
                 auto out = std::stringstream {};
                 out << &self;
                 return out.str();
             })
        .def("test_match", &ValueType::test_match, nb::rv_policy::copy, "constructor"_a, "grammar"_a)
        .def("accept", &ValueType::accept, "visitor"_a)
        .def("get_index", &ValueType::get_index)
        .def("get_head", &ValueType::get_head, nb::rv_policy::reference_internal)
        .def("get_body", &ValueType::get_body, nb::rv_policy::reference_internal);

    nb::bind_vector<cnf_grammar::SubstitutionRuleList<D>>(m, (class_name + "List").c_str());
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_cnf_grammar_concept_atomic_state(nb::module_& m, const std::string& class_name)
{
    using BaseType = cnf_grammar::IConstructor<ConceptTag>;
    using ValueType = cnf_grammar::ConceptAtomicStateImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_cnf_grammar_concept_atomic_goal(nb::module_& m, const std::string& class_name)
{
    using BaseType = cnf_grammar::IConstructor<ConceptTag>;
    using ValueType = cnf_grammar::ConceptAtomicGoalImpl<P>;

    nb::class_<ValueType, BaseType>(m,
                                    class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal)
        .def("get_polarity", &ValueType::get_polarity, nb::rv_policy::copy);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_cnf_grammar_role_atomic_state(nb::module_& m, const std::string& class_name)
{
    using BaseType = cnf_grammar::IConstructor<RoleTag>;
    using ValueType = cnf_grammar::RoleAtomicStateImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_cnf_grammar_role_atomic_goal(nb::module_& m, const std::string& class_name)
{
    using BaseType = cnf_grammar::IConstructor<RoleTag>;
    using ValueType = cnf_grammar::RoleAtomicGoalImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal)
        .def("get_polarity", &ValueType::get_polarity, nb::rv_policy::copy);
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
void bind_cnf_grammar_boolean_atomic_state(nb::module_& m, const std::string& class_name)
{
    using BaseType = cnf_grammar::IConstructor<BooleanTag>;
    using ValueType = cnf_grammar::BooleanAtomicStateImpl<P>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_predicate", &ValueType::get_predicate, nb::rv_policy::reference_internal);
};

template<IsConceptOrRoleTag D>
void bind_cnf_grammar_boolean_nonempty(nb::module_& m, const std::string& class_name)
{
    using BaseType = cnf_grammar::IConstructor<BooleanTag>;
    using ValueType = cnf_grammar::BooleanNonemptyImpl<D>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_constructor", &ValueType::get_nonterminal, nb::rv_policy::reference_internal);
};

template<IsConceptOrRoleTag D>
void bind_cnf_grammar_numerical_count(nb::module_& m, const std::string& class_name)
{
    using BaseType = cnf_grammar::IConstructor<NumericalTag>;
    using ValueType = cnf_grammar::NumericalCountImpl<D>;

    nb::class_<ValueType, BaseType>(m, class_name.c_str())  //
        .def("get_constructor", &ValueType::get_nonterminal, nb::rv_policy::reference_internal);
};
}

#endif