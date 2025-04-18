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

class IPyRefinementPruningFunction : public dl::IRefinementPruningFunction
{
public:
    NB_TRAMPOLINE(dl::IRefinementPruningFunction, 4);

    /* Trampoline (need one for each virtual function) */
    bool should_prune(dl::Constructor<dl::ConceptTag> constructor) override { NB_OVERRIDE_PURE(should_prune, constructor); }
    bool should_prune(dl::Constructor<dl::RoleTag> constructor) override { NB_OVERRIDE_PURE(should_prune, constructor); }
    bool should_prune(dl::Constructor<dl::BooleanTag> constructor) override { NB_OVERRIDE_PURE(should_prune, constructor); }
    bool should_prune(dl::Constructor<dl::NumericalTag> constructor) override { NB_OVERRIDE_PURE(should_prune, constructor); }
};

void bind_languages_description_logics(nb::module_& m)
{
    nb::enum_<dl::cnf_grammar::GrammarSpecificationEnum>(m, "GrammarSpecificationEnum")  //
        .value("FRANCES_ET_AL_AAAI2021", dl::cnf_grammar::GrammarSpecificationEnum::FRANCES_ET_AL_AAAI2021);

    bind_constructor<dl::ConceptTag>(m, "Concept");
    bind_constructor<dl::RoleTag>(m, "Role");
    bind_constructor<dl::BooleanTag>(m, "Boolean");
    bind_constructor<dl::NumericalTag>(m, "Numerical");

    nb::class_<dl::ConceptBotImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptBot");
    nb::class_<dl::ConceptTopImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptTop");
    bind_concept_atomic_state<formalism::StaticTag>(m, "ConceptStaticAtomicState");
    bind_concept_atomic_state<formalism::FluentTag>(m, "ConceptFluentAtomicState");
    bind_concept_atomic_state<formalism::DerivedTag>(m, "ConceptDerivedAtomicState");
    bind_concept_atomic_goal<formalism::StaticTag>(m, "ConceptStaticAtomicGoal");
    bind_concept_atomic_goal<formalism::FluentTag>(m, "ConceptFluentAtomicGoal");
    bind_concept_atomic_goal<formalism::DerivedTag>(m, "ConceptDerivedAtomicGoal");
    nb::class_<dl::ConceptIntersectionImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptIntersection")
        .def("get_left_concept", &dl::ConceptIntersectionImpl::get_left_concept, nb::rv_policy::reference_internal)
        .def("get_right_concept", &dl::ConceptIntersectionImpl::get_right_concept, nb::rv_policy::reference_internal);
    nb::class_<dl::ConceptUnionImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptUnion")
        .def("get_left_concept", &dl::ConceptUnionImpl::get_left_concept, nb::rv_policy::reference_internal)
        .def("get_right_concept", &dl::ConceptUnionImpl::get_right_concept, nb::rv_policy::reference_internal);
    nb::class_<dl::ConceptNegationImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptNegation")
        .def("get_concept", &dl::ConceptNegationImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<dl::ConceptValueRestrictionImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptValueRestriction")
        .def("get_role", &dl::ConceptValueRestrictionImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_concept", &dl::ConceptValueRestrictionImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<dl::ConceptExistentialQuantificationImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptExistentialQuantification")
        .def("get_role", &dl::ConceptExistentialQuantificationImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_concept", &dl::ConceptExistentialQuantificationImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<dl::ConceptRoleValueMapContainmentImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptRoleValueMapContainment")
        .def("get_left_role", &dl::ConceptRoleValueMapContainmentImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &dl::ConceptRoleValueMapContainmentImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<dl::ConceptRoleValueMapEqualityImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptRoleValueMapEquality")
        .def("get_left_role", &dl::ConceptRoleValueMapEqualityImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &dl::ConceptRoleValueMapEqualityImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<dl::ConceptNominalImpl, dl::IConstructor<dl::ConceptTag>>(m, "ConceptNominal")
        .def("get_object", &dl::ConceptNominalImpl::get_object, nb::rv_policy::reference_internal);

    nb::class_<dl::RoleUniversalImpl, dl::IConstructor<dl::RoleTag>>(m, "RoleUniversal");
    bind_role_atomic_state<formalism::StaticTag>(m, "RoleStaticAtomicState");
    bind_role_atomic_state<formalism::FluentTag>(m, "RoleFluentAtomicState");
    bind_role_atomic_state<formalism::DerivedTag>(m, "RoleDerivedAtomicState");
    bind_role_atomic_goal<formalism::StaticTag>(m, "RoleStaticAtomicGoal");
    bind_role_atomic_goal<formalism::FluentTag>(m, "RoleFluentAtomicGoal");
    bind_role_atomic_goal<formalism::DerivedTag>(m, "RoleDerivedAtomicGoal");
    nb::class_<dl::RoleIntersectionImpl, dl::IConstructor<dl::RoleTag>>(m, "RoleIntersection")
        .def("get_left_role", &dl::RoleIntersectionImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &dl::RoleIntersectionImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<dl::RoleUnionImpl, dl::IConstructor<dl::RoleTag>>(m, "RoleUnion")
        .def("get_left_role", &dl::RoleUnionImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &dl::RoleUnionImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<dl::RoleComplementImpl, dl::IConstructor<dl::RoleTag>>(m, "RoleComplement")
        .def("get_role", &dl::RoleComplementImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<dl::RoleInverseImpl, dl::IConstructor<dl::RoleTag>>(m, "RoleInverse")
        .def("get_role", &dl::RoleInverseImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<dl::RoleCompositionImpl, dl::IConstructor<dl::RoleTag>>(m, "RoleComposition")
        .def("get_left_role", &dl::RoleCompositionImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &dl::RoleCompositionImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<dl::RoleTransitiveClosureImpl, dl::IConstructor<dl::RoleTag>>(m, "RoleTransitiveClosure")
        .def("get_role", &dl::RoleTransitiveClosureImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<dl::RoleReflexiveTransitiveClosureImpl, dl::IConstructor<dl::RoleTag>>(m, "RoleReflexiveTransitiveClosure")
        .def("get_role", &dl::RoleReflexiveTransitiveClosureImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<dl::RoleRestrictionImpl, dl::IConstructor<dl::RoleTag>>(m, "RoleRestriction")
        .def("get_role", &dl::RoleRestrictionImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_concept", &dl::RoleRestrictionImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<dl::RoleIdentityImpl, dl::IConstructor<dl::RoleTag>>(m, "RoleIdentity")
        .def("get_concept", &dl::RoleIdentityImpl::get_concept, nb::rv_policy::reference_internal);

    bind_boolean_atomic_state<formalism::StaticTag>(m, "BooleanStaticAtomicState");
    bind_boolean_atomic_state<formalism::FluentTag>(m, "BooleanFluentAtomicState");
    bind_boolean_atomic_state<formalism::DerivedTag>(m, "BooleanDerivedAtomicState");
    bind_boolean_nonempty<dl::ConceptTag>(m, "BooleanConceptNonempty");
    bind_boolean_nonempty<dl::RoleTag>(m, "BooleanRoleNonempty");

    bind_numerical_count<dl::ConceptTag>(m, "NumericalConceptCount");
    bind_numerical_count<dl::RoleTag>(m, "NumericalRoleCount");
    nb::class_<dl::NumericalDistanceImpl, dl::IConstructor<dl::NumericalTag>>(m, "NumericalDistance")
        .def("get_left_concept", &dl::NumericalDistanceImpl::get_left_concept, nb::rv_policy::reference_internal)
        .def("get_role", &dl::NumericalDistanceImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_right_concept", &dl::NumericalDistanceImpl::get_right_concept, nb::rv_policy::reference_internal);

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
        .def("__str__", [](const dl::cnf_grammar::Grammar& self) { return to_string(self); })
        .def("accept", &dl::cnf_grammar::Grammar::accept, "visitor"_a)
        .def("test_match",
             [](const dl::cnf_grammar::Grammar& self, const dl::Constructor<dl::NumericalTag> constructor) { return self.test_match(constructor); })
        .def("test_match", [](const dl::cnf_grammar::Grammar& self, const dl::Constructor<dl::RoleTag> constructor) { return self.test_match(constructor); })
        .def("test_match", [](const dl::cnf_grammar::Grammar& self, const dl::Constructor<dl::BooleanTag> constructor) { return self.test_match(constructor); })
        .def("test_match",
             [](const dl::cnf_grammar::Grammar& self, const dl::Constructor<dl::NumericalTag> constructor) { return self.test_match(constructor); })

        .def("get_repositories", &dl::cnf_grammar::Grammar::get_repositories, nb::rv_policy::reference_internal)
        .def("get_domain", &dl::cnf_grammar::Grammar::get_domain, nb::rv_policy::reference_internal);

    nb::class_<dl::IRefinementPruningFunction, IPyRefinementPruningFunction>(m, "IRefinementPruningFunction")
        .def(nb::init<>())
        .def("should_prune", nb::overload_cast<dl::Constructor<dl::ConceptTag>>(&dl::IRefinementPruningFunction::should_prune), "concept"_a)
        .def("should_prune", nb::overload_cast<dl::Constructor<dl::RoleTag>>(&dl::IRefinementPruningFunction::should_prune), "role"_a)
        .def("should_prune", nb::overload_cast<dl::Constructor<dl::BooleanTag>>(&dl::IRefinementPruningFunction::should_prune), "boolean"_a)
        .def("should_prune", nb::overload_cast<dl::Constructor<dl::NumericalTag>>(&dl::IRefinementPruningFunction::should_prune), "numerical"_a);

    nb::class_<dl::StateListRefinementPruningFunction, dl::IRefinementPruningFunction>(m, "StateListRefinementPruningFunction")
        .def(nb::init<const mimir::datasets::GeneralizedStateSpace&>())
        .def(nb::init<const mimir::datasets::GeneralizedStateSpace&, const mimir::graphs::ClassGraph&>())
        .def(nb::init<mimir::formalism::ProblemMap<mimir::search::StateList>>());

    nb::class_<dl::cnf_grammar::GeneratedSentencesContainer>(m, "GeneratedSentencesContainer")  //
        .def(nb::init<>());

    nb::class_<dl::cnf_grammar::GeneratorVisitor>(m, "GeneratorVisitor")
        .def(nb::init<dl::IRefinementPruningFunction&, dl::cnf_grammar::GeneratedSentencesContainer&, dl::Repositories&, size_t>())
        .def("visit", nb::overload_cast<const dl::cnf_grammar::Grammar&>(&dl::cnf_grammar::GeneratorVisitor::visit));
}

}
