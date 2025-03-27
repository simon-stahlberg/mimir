#include "bindings.hpp"

#include "../../init_declarations.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/trampoline.h>

using namespace mimir;
using namespace mimir::languages;
using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::bindings
{

class IPyDLVisitor : public dl::IVisitor
{
public:
    NB_TRAMPOLINE(dl::IVisitor, 40);

    /* Trampoline (need one for each virtual function) */
    void visit(dl::ConceptBot constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptTop constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptAtomicState<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptAtomicState<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptAtomicState<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptAtomicGoal<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptAtomicGoal<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptAtomicGoal<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptIntersection constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptUnion constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptNegation constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptValueRestriction constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptExistentialQuantification constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptRoleValueMapContainment constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptRoleValueMapEquality constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::ConceptNominal constructor) override { NB_OVERRIDE_PURE(visit, constructor); }

    void visit(dl::RoleUniversal constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleAtomicState<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleAtomicState<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleAtomicState<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleAtomicGoal<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleAtomicGoal<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleAtomicGoal<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleIntersection constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleUnion constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleComplement constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleInverse constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleComposition constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleTransitiveClosure constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleReflexiveTransitiveClosure constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleRestriction constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::RoleIdentity constructor) override { NB_OVERRIDE_PURE(visit, constructor); }

    void visit(dl::BooleanAtomicState<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::BooleanAtomicState<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::BooleanAtomicState<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::BooleanNonempty<dl::ConceptTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::BooleanNonempty<dl::RoleTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }

    void visit(dl::NumericalCount<dl::ConceptTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::NumericalCount<dl::RoleTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(dl::NumericalDistance constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
};

void bind_languages_description_logics(nb::module_& m)
{
    nb::enum_<dl::cnf_grammar::GrammarSpecificationEnum>(m, "GrammarSpecificationEnum")  //
        .value("FRANCES_ET_AL_AAAI2021", dl::cnf_grammar::GrammarSpecificationEnum::FRANCES_ET_AL_AAAI2021);

    bind_constructor<dl::ConceptTag>(m, "ConceptConstructor");
    bind_constructor<dl::RoleTag>(m, "RoleConstructor");
    bind_constructor<dl::BooleanTag>(m, "BooleanConstructor");
    bind_constructor<dl::NumericalTag>(m, "NumericalConstructor");

    nb::class_<dl::ConceptBotImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptBotConstructor");
    nb::class_<dl::ConceptTopImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptTopConstructor");

    nb::class_<dl::Repositories>(m, "Repositories")
        .def(nb::init<>())
        .def("get_or_create_concept", &dl::Repositories::get_or_create<dl::ConceptTag>, "sentence"_a, "domain"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role", &dl::Repositories::get_or_create<dl::RoleTag>, "sentence"_a, "domain"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_boolean", &dl::Repositories::get_or_create<dl::BooleanTag>, "sentence"_a, "domain"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_numerical", &dl::Repositories::get_or_create<dl::NumericalTag>, "sentence"_a, "domain"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_bot", &dl::Repositories::get_or_create_concept_bot, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_top", &dl::Repositories::get_or_create_concept_top, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_intersection",
             &dl::Repositories::get_or_create_concept_intersection,
             "left_concept"_a,
             "right_concept"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_union",
             &dl::Repositories::get_or_create_concept_union,
             "left_concept"_a,
             "right_concept"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_negation", &dl::Repositories::get_or_create_concept_negation, "concept_"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_value_restriction",
             &dl::Repositories::get_or_create_concept_value_restriction,
             "role"_a,
             "concept_"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_existential_quantification",
             &dl::Repositories::get_or_create_concept_existential_quantification,
             "role"_a,
             "concept_"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_role_value_map_containment",
             &dl::Repositories::get_or_create_concept_role_value_map_containment,
             "left_role"_a,
             "right_role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_role_value_map_equality",
             &dl::Repositories::get_or_create_concept_role_value_map_equality,
             "left_role"_a,
             "right_role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_nominal", &dl::Repositories::get_or_create_concept_nominal, "object"_a, nb::rv_policy::reference_internal)

        /* Roles */
        .def("get_or_create_role_universal", &dl::Repositories::get_or_create_role_universal, nb::rv_policy::reference_internal)
        .def("get_or_create_role_intersection",
             &dl::Repositories::get_or_create_role_intersection,
             "left_role"_a,
             "right_role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_union", &dl::Repositories::get_or_create_role_union, "left_role"_a, "right_role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_complement", &dl::Repositories::get_or_create_role_complement, "role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_inverse", &dl::Repositories::get_or_create_role_inverse, "role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_composition",
             &dl::Repositories::get_or_create_role_composition,
             "left_role"_a,
             "right_role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_transitive_closure", &dl::Repositories::get_or_create_role_transitive_closure, "role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_reflexive_transitive_closure",
             &dl::Repositories::get_or_create_role_reflexive_transitive_closure,
             "role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_restriction", &dl::Repositories::get_or_create_role_restriction, "role"_a, "concept_"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_identity", &dl::Repositories::get_or_create_role_identity, "concept_"_a, nb::rv_policy::reference_internal)

        .def("get_or_create_concept_atomic_state_static",
             &dl::Repositories::get_or_create_concept_atomic_state<StaticTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_state_fluent",
             &dl::Repositories::get_or_create_concept_atomic_state<FluentTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_state_derived",
             &dl::Repositories::get_or_create_concept_atomic_state<DerivedTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)

        .def("get_or_create_concept_atomic_goal_static",
             &dl::Repositories::get_or_create_concept_atomic_goal<StaticTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_goal_fluent",
             &dl::Repositories::get_or_create_concept_atomic_goal<FluentTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_goal_derived", &dl::Repositories::get_or_create_concept_atomic_goal<DerivedTag>, "predicate"_a, "polarity"_a)

        .def("get_or_create_role_atomic_state_static",
             &dl::Repositories::get_or_create_role_atomic_state<StaticTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_state_fluent",
             &dl::Repositories::get_or_create_role_atomic_state<FluentTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_state_derived",
             &dl::Repositories::get_or_create_role_atomic_state<DerivedTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)

        .def("get_or_create_role_atomic_goal_static",
             &dl::Repositories::get_or_create_role_atomic_goal<StaticTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_goal_fluent",
             &dl::Repositories::get_or_create_role_atomic_goal<FluentTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_goal_derived",
             &dl::Repositories::get_or_create_role_atomic_goal<DerivedTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_atomic_state_static",
             &dl::Repositories::get_or_create_boolean_atomic_state<StaticTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_atomic_state_fluent",
             &dl::Repositories::get_or_create_boolean_atomic_state<FluentTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_atomic_state_derived",
             &dl::Repositories::get_or_create_boolean_atomic_state<DerivedTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_nonempty_concept",
             &dl::Repositories::get_or_create_boolean_nonempty<dl::ConceptTag>,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_nonempty_role",
             &dl::Repositories::get_or_create_boolean_nonempty<dl::RoleTag>,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_count_concept",
             &dl::Repositories::get_or_create_numerical_count<dl::ConceptTag>,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_count_role",
             &dl::Repositories::get_or_create_numerical_count<dl::RoleTag>,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_distance",
             &dl::Repositories::get_or_create_numerical_distance,
             "left_concept"_a,
             "role"_a,
             "right_concept"_a,
             nb::rv_policy::reference_internal);

    /* ConstructorVisitor */
    nb::class_<dl::IVisitor, IPyDLVisitor>(m, "ConstructorVisitor")  //
        .def(nb::init<>())
        .def("visit", nb::overload_cast<dl::ConceptBot>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptTop>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptAtomicState<formalism::StaticTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptAtomicState<formalism::FluentTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptAtomicState<formalism::DerivedTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptAtomicGoal<formalism::StaticTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptAtomicGoal<formalism::FluentTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptAtomicGoal<formalism::DerivedTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptIntersection>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptUnion>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptNegation>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptValueRestriction>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptExistentialQuantification>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptRoleValueMapContainment>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptRoleValueMapEquality>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::ConceptNominal>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleUniversal>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleAtomicState<formalism::StaticTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleAtomicState<formalism::FluentTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleAtomicState<formalism::DerivedTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleAtomicGoal<formalism::StaticTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleAtomicGoal<formalism::FluentTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleAtomicGoal<formalism::DerivedTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleIntersection>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleUnion>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleComplement>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleInverse>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleComposition>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleTransitiveClosure>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleReflexiveTransitiveClosure>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleRestriction>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::RoleIdentity>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::BooleanAtomicState<formalism::StaticTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::BooleanAtomicState<formalism::FluentTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::BooleanAtomicState<formalism::DerivedTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::BooleanNonempty<dl::ConceptTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::BooleanNonempty<dl::RoleTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::NumericalCount<dl::ConceptTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::NumericalCount<dl::RoleTag>>(&dl::IVisitor::visit))
        .def("visit", nb::overload_cast<dl::NumericalDistance>(&dl::IVisitor::visit));

    nb::class_<dl::cnf_grammar::Grammar>(m, "CNFGrammar")
        .def(nb::init<const std::string&, formalism::Domain>(), "bnf_description"_a, "domain"_a)

        .def_static("create", &dl::cnf_grammar::Grammar::create, "type"_a, "domain"_a)

        .def("accept", &dl::cnf_grammar::Grammar::accept, "visitor"_a)

        .def("test_match",
             [](const dl::cnf_grammar::Grammar& self, const dl::Constructor<dl::NumericalTag> constructor) { return self.test_match(constructor); })
        .def("test_match", [](const dl::cnf_grammar::Grammar& self, const dl::Constructor<dl::RoleTag> constructor) { return self.test_match(constructor); })
        .def("test_match", [](const dl::cnf_grammar::Grammar& self, const dl::Constructor<dl::BooleanTag> constructor) { return self.test_match(constructor); })
        .def("test_match",
             [](const dl::cnf_grammar::Grammar& self, const dl::Constructor<dl::NumericalTag> constructor) { return self.test_match(constructor); })

        .def("get_repositories", &dl::cnf_grammar::Grammar::get_repositories, nb::rv_policy::reference_internal)
        .def("get_domain", &dl::cnf_grammar::Grammar::get_domain, nb::rv_policy::reference_internal);
}

}
