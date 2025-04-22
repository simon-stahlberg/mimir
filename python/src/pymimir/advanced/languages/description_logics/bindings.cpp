#include "bindings.hpp"

#include "../../init_declarations.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/trampoline.h>

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::languages::dl
{

class IPyVisitor : public IVisitor
{
public:
    NB_TRAMPOLINE(IVisitor, 40);

    /* Trampoline (need one for each virtual function) */
    void visit(ConceptBot constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptTop constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptAtomicState<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptAtomicState<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptAtomicState<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptAtomicGoal<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptAtomicGoal<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptAtomicGoal<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptIntersection constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptUnion constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptNegation constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptValueRestriction constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptExistentialQuantification constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptRoleValueMapContainment constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptRoleValueMapEquality constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(ConceptNominal constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleUniversal constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleAtomicState<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleAtomicState<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleAtomicState<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleAtomicGoal<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleAtomicGoal<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleAtomicGoal<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleIntersection constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleUnion constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleComplement constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleInverse constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleComposition constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleTransitiveClosure constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleReflexiveTransitiveClosure constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleRestriction constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(RoleIdentity constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(BooleanAtomicState<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(BooleanAtomicState<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(BooleanAtomicState<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(BooleanNonempty<ConceptTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(BooleanNonempty<RoleTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(NumericalCount<ConceptTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(NumericalCount<RoleTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(NumericalDistance constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
};

class IPyRefinementPruningFunction : public IRefinementPruningFunction
{
public:
    NB_TRAMPOLINE(IRefinementPruningFunction, 4);

    /* Trampoline (need one for each virtual function) */
    bool should_prune(Constructor<ConceptTag> constructor) override { NB_OVERRIDE_PURE(should_prune, constructor); }
    bool should_prune(Constructor<RoleTag> constructor) override { NB_OVERRIDE_PURE(should_prune, constructor); }
    bool should_prune(Constructor<BooleanTag> constructor) override { NB_OVERRIDE_PURE(should_prune, constructor); }
    bool should_prune(Constructor<NumericalTag> constructor) override { NB_OVERRIDE_PURE(should_prune, constructor); }
};

namespace cnf_grammar
{
class IPyVisitor : public cnf_grammar::IVisitor
{
public:
    NB_TRAMPOLINE(cnf_grammar::IVisitor, 53);

    /* Trampoline (need one for each virtual function) */
    void visit(cnf_grammar::ConceptBot constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptTop constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptAtomicState<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptAtomicState<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptAtomicState<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptAtomicGoal<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptAtomicGoal<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptAtomicGoal<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptIntersection constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptUnion constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptNegation constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptValueRestriction constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptExistentialQuantification constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptRoleValueMapContainment constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptRoleValueMapEquality constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::ConceptNominal constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleUniversal constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleAtomicState<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleAtomicState<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleAtomicState<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleAtomicGoal<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleAtomicGoal<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleAtomicGoal<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleIntersection constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleUnion constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleComplement constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleInverse constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleComposition constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleTransitiveClosure constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleReflexiveTransitiveClosure constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleRestriction constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::RoleIdentity constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::BooleanAtomicState<formalism::StaticTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::BooleanAtomicState<formalism::FluentTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::BooleanAtomicState<formalism::DerivedTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::BooleanNonempty<ConceptTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::BooleanNonempty<RoleTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::NumericalCount<ConceptTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::NumericalCount<RoleTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::NumericalDistance constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(cnf_grammar::NonTerminal<ConceptTag> nonterminal) override { NB_OVERRIDE_PURE(visit, nonterminal); }
    void visit(cnf_grammar::NonTerminal<RoleTag> nonterminal) override { NB_OVERRIDE_PURE(visit, nonterminal); }
    void visit(cnf_grammar::NonTerminal<BooleanTag> nonterminal) override { NB_OVERRIDE_PURE(visit, nonterminal); }
    void visit(cnf_grammar::NonTerminal<NumericalTag> nonterminal) override { NB_OVERRIDE_PURE(visit, nonterminal); }
    void visit(cnf_grammar::DerivationRule<ConceptTag> derivation_rule) override { NB_OVERRIDE_PURE(visit, derivation_rule); }
    void visit(cnf_grammar::DerivationRule<RoleTag> derivation_rule) override { NB_OVERRIDE_PURE(visit, derivation_rule); }
    void visit(cnf_grammar::DerivationRule<BooleanTag> derivation_rule) override { NB_OVERRIDE_PURE(visit, derivation_rule); }
    void visit(cnf_grammar::DerivationRule<NumericalTag> derivation_rule) override { NB_OVERRIDE_PURE(visit, derivation_rule); }
    void visit(cnf_grammar::SubstitutionRule<ConceptTag> substitution_rule) override { NB_OVERRIDE_PURE(visit, substitution_rule); }
    void visit(cnf_grammar::SubstitutionRule<RoleTag> substitution_rule) override { NB_OVERRIDE_PURE(visit, substitution_rule); }
    void visit(cnf_grammar::SubstitutionRule<BooleanTag> substitution_rule) override { NB_OVERRIDE_PURE(visit, substitution_rule); }
    void visit(cnf_grammar::SubstitutionRule<NumericalTag> substitution_rule) override { NB_OVERRIDE_PURE(visit, substitution_rule); }

    void visit(const cnf_grammar::Grammar& grammar) override { NB_OVERRIDE_PURE(visit, grammar); }
};
}

void bind_module_definitions(nb::module_& m)
{
    nb::enum_<cnf_grammar::GrammarSpecificationEnum>(m, "GrammarSpecificationEnum")  //
        .value("FRANCES_ET_AL_AAAI2021", cnf_grammar::GrammarSpecificationEnum::FRANCES_ET_AL_AAAI2021);

    /**
     * DL
     */

    bind_constructor<ConceptTag>(m, "Concept");
    bind_constructor<RoleTag>(m, "Role");
    bind_constructor<BooleanTag>(m, "Boolean");
    bind_constructor<NumericalTag>(m, "Numerical");

    nb::class_<ConceptBotImpl, IConstructor<ConceptTag>>(m, "ConceptBot");
    nb::class_<ConceptTopImpl, IConstructor<ConceptTag>>(m, "ConceptTop");
    bind_concept_atomic_state<formalism::StaticTag>(m, "ConceptStaticAtomicState");
    bind_concept_atomic_state<formalism::FluentTag>(m, "ConceptFluentAtomicState");
    bind_concept_atomic_state<formalism::DerivedTag>(m, "ConceptDerivedAtomicState");
    bind_concept_atomic_goal<formalism::StaticTag>(m, "ConceptStaticAtomicGoal");
    bind_concept_atomic_goal<formalism::FluentTag>(m, "ConceptFluentAtomicGoal");
    bind_concept_atomic_goal<formalism::DerivedTag>(m, "ConceptDerivedAtomicGoal");
    nb::class_<ConceptIntersectionImpl, IConstructor<ConceptTag>>(m, "ConceptIntersection")
        .def("get_left_concept", &ConceptIntersectionImpl::get_left_concept, nb::rv_policy::reference_internal)
        .def("get_right_concept", &ConceptIntersectionImpl::get_right_concept, nb::rv_policy::reference_internal);
    nb::class_<ConceptUnionImpl, IConstructor<ConceptTag>>(m, "ConceptUnion")
        .def("get_left_concept", &ConceptUnionImpl::get_left_concept, nb::rv_policy::reference_internal)
        .def("get_right_concept", &ConceptUnionImpl::get_right_concept, nb::rv_policy::reference_internal);
    nb::class_<ConceptNegationImpl, IConstructor<ConceptTag>>(m, "ConceptNegation")
        .def("get_concept", &ConceptNegationImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<ConceptValueRestrictionImpl, IConstructor<ConceptTag>>(m, "ConceptValueRestriction")
        .def("get_role", &ConceptValueRestrictionImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_concept", &ConceptValueRestrictionImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<ConceptExistentialQuantificationImpl, IConstructor<ConceptTag>>(m, "ConceptExistentialQuantification")
        .def("get_role", &ConceptExistentialQuantificationImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_concept", &ConceptExistentialQuantificationImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<ConceptRoleValueMapContainmentImpl, IConstructor<ConceptTag>>(m, "ConceptRoleValueMapContainment")
        .def("get_left_role", &ConceptRoleValueMapContainmentImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &ConceptRoleValueMapContainmentImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<ConceptRoleValueMapEqualityImpl, IConstructor<ConceptTag>>(m, "ConceptRoleValueMapEquality")
        .def("get_left_role", &ConceptRoleValueMapEqualityImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &ConceptRoleValueMapEqualityImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<ConceptNominalImpl, IConstructor<ConceptTag>>(m, "ConceptNominal")
        .def("get_object", &ConceptNominalImpl::get_object, nb::rv_policy::reference_internal);

    nb::class_<RoleUniversalImpl, IConstructor<RoleTag>>(m, "RoleUniversal");
    bind_role_atomic_state<formalism::StaticTag>(m, "RoleStaticAtomicState");
    bind_role_atomic_state<formalism::FluentTag>(m, "RoleFluentAtomicState");
    bind_role_atomic_state<formalism::DerivedTag>(m, "RoleDerivedAtomicState");
    bind_role_atomic_goal<formalism::StaticTag>(m, "RoleStaticAtomicGoal");
    bind_role_atomic_goal<formalism::FluentTag>(m, "RoleFluentAtomicGoal");
    bind_role_atomic_goal<formalism::DerivedTag>(m, "RoleDerivedAtomicGoal");
    nb::class_<RoleIntersectionImpl, IConstructor<RoleTag>>(m, "RoleIntersection")
        .def("get_left_role", &RoleIntersectionImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &RoleIntersectionImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<RoleUnionImpl, IConstructor<RoleTag>>(m, "RoleUnion")
        .def("get_left_role", &RoleUnionImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &RoleUnionImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<RoleComplementImpl, IConstructor<RoleTag>>(m, "RoleComplement")
        .def("get_role", &RoleComplementImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<RoleInverseImpl, IConstructor<RoleTag>>(m, "RoleInverse").def("get_role", &RoleInverseImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<RoleCompositionImpl, IConstructor<RoleTag>>(m, "RoleComposition")
        .def("get_left_role", &RoleCompositionImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &RoleCompositionImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<RoleTransitiveClosureImpl, IConstructor<RoleTag>>(m, "RoleTransitiveClosure")
        .def("get_role", &RoleTransitiveClosureImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<RoleReflexiveTransitiveClosureImpl, IConstructor<RoleTag>>(m, "RoleReflexiveTransitiveClosure")
        .def("get_role", &RoleReflexiveTransitiveClosureImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<RoleRestrictionImpl, IConstructor<RoleTag>>(m, "RoleRestriction")
        .def("get_role", &RoleRestrictionImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_concept", &RoleRestrictionImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<RoleIdentityImpl, IConstructor<RoleTag>>(m, "RoleIdentity")
        .def("get_concept", &RoleIdentityImpl::get_concept, nb::rv_policy::reference_internal);

    bind_boolean_atomic_state<formalism::StaticTag>(m, "BooleanStaticAtomicState");
    bind_boolean_atomic_state<formalism::FluentTag>(m, "BooleanFluentAtomicState");
    bind_boolean_atomic_state<formalism::DerivedTag>(m, "BooleanDerivedAtomicState");
    bind_boolean_nonempty<ConceptTag>(m, "BooleanConceptNonempty");
    bind_boolean_nonempty<RoleTag>(m, "BooleanRoleNonempty");

    bind_numerical_count<ConceptTag>(m, "NumericalConceptCount");
    bind_numerical_count<RoleTag>(m, "NumericalRoleCount");
    nb::class_<NumericalDistanceImpl, IConstructor<NumericalTag>>(m, "NumericalDistance")
        .def("get_left_concept", &NumericalDistanceImpl::get_left_concept, nb::rv_policy::reference_internal)
        .def("get_role", &NumericalDistanceImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_right_concept", &NumericalDistanceImpl::get_right_concept, nb::rv_policy::reference_internal);

    nb::class_<Repositories>(m, "Repositories")
        .def(nb::init<>())
        .def("get_or_create_concept", &Repositories::get_or_create<ConceptTag>, "sentence"_a, "domain"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role", &Repositories::get_or_create<RoleTag>, "sentence"_a, "domain"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_boolean", &Repositories::get_or_create<BooleanTag>, "sentence"_a, "domain"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_numerical", &Repositories::get_or_create<NumericalTag>, "sentence"_a, "domain"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_bot", &Repositories::get_or_create_concept_bot, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_top", &Repositories::get_or_create_concept_top, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_intersection",
             &Repositories::get_or_create_concept_intersection,
             "left_concept"_a,
             "right_concept"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_union", &Repositories::get_or_create_concept_union, "left_concept"_a, "right_concept"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_negation", &Repositories::get_or_create_concept_negation, "concept_"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_value_restriction",
             &Repositories::get_or_create_concept_value_restriction,
             "role"_a,
             "concept_"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_existential_quantification",
             &Repositories::get_or_create_concept_existential_quantification,
             "role"_a,
             "concept_"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_role_value_map_containment",
             &Repositories::get_or_create_concept_role_value_map_containment,
             "left_role"_a,
             "right_role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_role_value_map_equality",
             &Repositories::get_or_create_concept_role_value_map_equality,
             "left_role"_a,
             "right_role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_nominal", &Repositories::get_or_create_concept_nominal, "object"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_universal", &Repositories::get_or_create_role_universal, nb::rv_policy::reference_internal)
        .def("get_or_create_role_intersection",
             &Repositories::get_or_create_role_intersection,
             "left_role"_a,
             "right_role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_union", &Repositories::get_or_create_role_union, "left_role"_a, "right_role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_complement", &Repositories::get_or_create_role_complement, "role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_inverse", &Repositories::get_or_create_role_inverse, "role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_composition", &Repositories::get_or_create_role_composition, "left_role"_a, "right_role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_transitive_closure", &Repositories::get_or_create_role_transitive_closure, "role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_reflexive_transitive_closure",
             &Repositories::get_or_create_role_reflexive_transitive_closure,
             "role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_restriction", &Repositories::get_or_create_role_restriction, "role"_a, "concept_"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_identity", &Repositories::get_or_create_role_identity, "concept_"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_state_static",
             &Repositories::get_or_create_concept_atomic_state<StaticTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_state_fluent",
             &Repositories::get_or_create_concept_atomic_state<FluentTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_state_derived",
             &Repositories::get_or_create_concept_atomic_state<DerivedTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_goal_static",
             &Repositories::get_or_create_concept_atomic_goal<StaticTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_goal_fluent",
             &Repositories::get_or_create_concept_atomic_goal<FluentTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_goal_derived", &Repositories::get_or_create_concept_atomic_goal<DerivedTag>, "predicate"_a, "polarity"_a)
        .def("get_or_create_role_atomic_state_static",
             &Repositories::get_or_create_role_atomic_state<StaticTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_state_fluent",
             &Repositories::get_or_create_role_atomic_state<FluentTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_state_derived",
             &Repositories::get_or_create_role_atomic_state<DerivedTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_goal_static",
             &Repositories::get_or_create_role_atomic_goal<StaticTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_goal_fluent",
             &Repositories::get_or_create_role_atomic_goal<FluentTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_goal_derived",
             &Repositories::get_or_create_role_atomic_goal<DerivedTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_atomic_state_static",
             &Repositories::get_or_create_boolean_atomic_state<StaticTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_atomic_state_fluent",
             &Repositories::get_or_create_boolean_atomic_state<FluentTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_atomic_state_derived",
             &Repositories::get_or_create_boolean_atomic_state<DerivedTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_nonempty_concept",
             &Repositories::get_or_create_boolean_nonempty<ConceptTag>,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_nonempty_role", &Repositories::get_or_create_boolean_nonempty<RoleTag>, "constructor"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_count_concept",
             &Repositories::get_or_create_numerical_count<ConceptTag>,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_count_role", &Repositories::get_or_create_numerical_count<RoleTag>, "constructor"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_distance",
             &Repositories::get_or_create_numerical_distance,
             "left_concept"_a,
             "role"_a,
             "right_concept"_a,
             nb::rv_policy::reference_internal);

    /* ConstructorVisitor */
    nb::class_<IVisitor, IPyVisitor>(m, "ConstructorVisitor")  //
        .def(nb::init<>())
        .def("visit", nb::overload_cast<ConceptBot>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptTop>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptAtomicState<formalism::StaticTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptAtomicState<formalism::FluentTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptAtomicState<formalism::DerivedTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptAtomicGoal<formalism::StaticTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptAtomicGoal<formalism::FluentTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptAtomicGoal<formalism::DerivedTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptIntersection>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptUnion>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptNegation>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptValueRestriction>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptExistentialQuantification>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptRoleValueMapContainment>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptRoleValueMapEquality>(&IVisitor::visit))
        .def("visit", nb::overload_cast<ConceptNominal>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleUniversal>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleAtomicState<formalism::StaticTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleAtomicState<formalism::FluentTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleAtomicState<formalism::DerivedTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleAtomicGoal<formalism::StaticTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleAtomicGoal<formalism::FluentTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleAtomicGoal<formalism::DerivedTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleIntersection>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleUnion>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleComplement>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleInverse>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleComposition>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleTransitiveClosure>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleReflexiveTransitiveClosure>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleRestriction>(&IVisitor::visit))
        .def("visit", nb::overload_cast<RoleIdentity>(&IVisitor::visit))
        .def("visit", nb::overload_cast<BooleanAtomicState<formalism::StaticTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<BooleanAtomicState<formalism::FluentTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<BooleanAtomicState<formalism::DerivedTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<BooleanNonempty<ConceptTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<BooleanNonempty<RoleTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<NumericalCount<ConceptTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<NumericalCount<RoleTag>>(&IVisitor::visit))
        .def("visit", nb::overload_cast<NumericalDistance>(&IVisitor::visit));

    nb::class_<DenotationImpl<ConceptTag>>(m, "ConceptDenotation");

    nb::class_<DenotationImpl<RoleTag>>(m, "RoleDenotation");

    nb::class_<DenotationImpl<BooleanTag>>(m, "BooleanDenotation")  //
        .def("get_data", nb::overload_cast<>(&DenotationImpl<BooleanTag>::get_data, nb::const_), nb::rv_policy::copy);

    nb::class_<DenotationImpl<NumericalTag>>(m, "NumericalDenotation")
        .def("get_data", nb::overload_cast<>(&DenotationImpl<NumericalTag>::get_data, nb::const_), nb::rv_policy::copy);

    nb::class_<DenotationRepositories>(m, "DenotationRepositories")  //
        .def(nb::init<>());

    nb::class_<EvaluationContext>(m, "EvaluationContext")  //
        .def(nb::init<search::State, formalism::Problem, DenotationRepositories&>(), "state"_a, "problem"_a, "denotation_repositories"_a);

    /**
     * CNFGrammar
     */

    cnf_grammar::bind_constructor<ConceptTag>(m, "CNFConcept");
    cnf_grammar::bind_constructor<RoleTag>(m, "CNFRole");
    cnf_grammar::bind_constructor<BooleanTag>(m, "CNFBoolean");
    cnf_grammar::bind_constructor<NumericalTag>(m, "CNFNumerical");

    cnf_grammar::bind_nonterminal<ConceptTag>(m, "CNFConceptNonTerminal");
    cnf_grammar::bind_nonterminal<RoleTag>(m, "CNFRoleNonTerminal");
    cnf_grammar::bind_nonterminal<BooleanTag>(m, "CNFBooleanNonTerminal");
    cnf_grammar::bind_nonterminal<NumericalTag>(m, "CNFNumericalNonTerminal");

    cnf_grammar::bind_derivation_rule<ConceptTag>(m, "CNFConceptDerivationRule");
    cnf_grammar::bind_derivation_rule<RoleTag>(m, "CNFRoleDerivationRule");
    cnf_grammar::bind_derivation_rule<BooleanTag>(m, "CNFBooleanDerivationRule");
    cnf_grammar::bind_derivation_rule<NumericalTag>(m, "CNFNumericalDerivationRule");

    cnf_grammar::bind_substitution_rule<ConceptTag>(m, "CNFConceptSubstitutionRule");
    cnf_grammar::bind_substitution_rule<RoleTag>(m, "CNFRoleSubstitutionRule");
    cnf_grammar::bind_substitution_rule<BooleanTag>(m, "CNFBooleanSubstitutionRule");
    cnf_grammar::bind_substitution_rule<NumericalTag>(m, "CNFNumericalSubstitutionRule");

    nb::class_<cnf_grammar::ConceptBotImpl, cnf_grammar::IConstructor<ConceptTag>>(m, "CNFConceptBot");
    nb::class_<cnf_grammar::ConceptTopImpl, cnf_grammar::IConstructor<ConceptTag>>(m, "CNFConceptTop");
    cnf_grammar::bind_concept_atomic_state<formalism::StaticTag>(m, "CNFConceptStaticAtomicState");
    cnf_grammar::bind_concept_atomic_state<formalism::FluentTag>(m, "CNFConceptFluentAtomicState");
    cnf_grammar::bind_concept_atomic_state<formalism::DerivedTag>(m, "CNFConceptDerivedAtomicState");
    cnf_grammar::bind_concept_atomic_goal<formalism::StaticTag>(m, "CNFConceptStaticAtomicGoal");
    cnf_grammar::bind_concept_atomic_goal<formalism::FluentTag>(m, "CNFConceptFluentAtomicGoal");
    cnf_grammar::bind_concept_atomic_goal<formalism::DerivedTag>(m, "CNFConceptDerivedAtomicGoal");
    nb::class_<cnf_grammar::ConceptIntersectionImpl, cnf_grammar::IConstructor<ConceptTag>>(m, "CNFConceptIntersection")
        .def("get_left_concept", &cnf_grammar::ConceptIntersectionImpl::get_left_concept, nb::rv_policy::reference_internal)
        .def("get_right_concept", &cnf_grammar::ConceptIntersectionImpl::get_right_concept, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::ConceptUnionImpl, cnf_grammar::IConstructor<ConceptTag>>(m, "CNFConceptUnion")
        .def("get_left_concept", &cnf_grammar::ConceptUnionImpl::get_left_concept, nb::rv_policy::reference_internal)
        .def("get_right_concept", &cnf_grammar::ConceptUnionImpl::get_right_concept, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::ConceptNegationImpl, cnf_grammar::IConstructor<ConceptTag>>(m, "CNFConceptNegation")
        .def("get_concept", &cnf_grammar::ConceptNegationImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::ConceptValueRestrictionImpl, cnf_grammar::IConstructor<ConceptTag>>(m, "CNFConceptValueRestriction")
        .def("get_role", &cnf_grammar::ConceptValueRestrictionImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_concept", &cnf_grammar::ConceptValueRestrictionImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::ConceptExistentialQuantificationImpl, cnf_grammar::IConstructor<ConceptTag>>(m, "CNFConceptExistentialQuantification")
        .def("get_role", &cnf_grammar::ConceptExistentialQuantificationImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_concept", &cnf_grammar::ConceptExistentialQuantificationImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::ConceptRoleValueMapContainmentImpl, cnf_grammar::IConstructor<ConceptTag>>(m, "CNFConceptRoleValueMapContainment")
        .def("get_left_role", &cnf_grammar::ConceptRoleValueMapContainmentImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &cnf_grammar::ConceptRoleValueMapContainmentImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::ConceptRoleValueMapEqualityImpl, cnf_grammar::IConstructor<ConceptTag>>(m, "CNFConceptRoleValueMapEquality")
        .def("get_left_role", &cnf_grammar::ConceptRoleValueMapEqualityImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &cnf_grammar::ConceptRoleValueMapEqualityImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::ConceptNominalImpl, cnf_grammar::IConstructor<ConceptTag>>(m, "CNFConceptNominal")
        .def("get_object", &cnf_grammar::ConceptNominalImpl::get_object, nb::rv_policy::reference_internal);

    nb::class_<cnf_grammar::RoleUniversalImpl, cnf_grammar::IConstructor<RoleTag>>(m, "CNFRoleUniversal");
    cnf_grammar::bind_role_atomic_state<formalism::StaticTag>(m, "CNFRoleStaticAtomicState");
    cnf_grammar::bind_role_atomic_state<formalism::FluentTag>(m, "CNFRoleFluentAtomicState");
    cnf_grammar::bind_role_atomic_state<formalism::DerivedTag>(m, "CNFRoleDerivedAtomicState");
    cnf_grammar::bind_role_atomic_goal<formalism::StaticTag>(m, "CNFRoleStaticAtomicGoal");
    cnf_grammar::bind_role_atomic_goal<formalism::FluentTag>(m, "CNFRoleFluentAtomicGoal");
    cnf_grammar::bind_role_atomic_goal<formalism::DerivedTag>(m, "CNFRoleDerivedAtomicGoal");
    nb::class_<cnf_grammar::RoleIntersectionImpl, cnf_grammar::IConstructor<RoleTag>>(m, "CNFRoleIntersection")
        .def("get_left_role", &cnf_grammar::RoleIntersectionImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &cnf_grammar::RoleIntersectionImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::RoleUnionImpl, cnf_grammar::IConstructor<RoleTag>>(m, "CNFRoleUnion")
        .def("get_left_role", &cnf_grammar::RoleUnionImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &cnf_grammar::RoleUnionImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::RoleComplementImpl, cnf_grammar::IConstructor<RoleTag>>(m, "CNFRoleComplement")
        .def("get_role", &cnf_grammar::RoleComplementImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::RoleInverseImpl, cnf_grammar::IConstructor<RoleTag>>(m, "CNFRoleInverse")
        .def("get_role", &cnf_grammar::RoleInverseImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::RoleCompositionImpl, cnf_grammar::IConstructor<RoleTag>>(m, "CNFRoleComposition")
        .def("get_left_role", &cnf_grammar::RoleCompositionImpl::get_left_role, nb::rv_policy::reference_internal)
        .def("get_right_role", &cnf_grammar::RoleCompositionImpl::get_right_role, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::RoleTransitiveClosureImpl, cnf_grammar::IConstructor<RoleTag>>(m, "CNFRoleTransitiveClosure")
        .def("get_role", &cnf_grammar::RoleTransitiveClosureImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::RoleReflexiveTransitiveClosureImpl, cnf_grammar::IConstructor<RoleTag>>(m, "CNFRoleReflexiveTransitiveClosure")
        .def("get_role", &cnf_grammar::RoleReflexiveTransitiveClosureImpl::get_role, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::RoleRestrictionImpl, cnf_grammar::IConstructor<RoleTag>>(m, "CNFRoleRestriction")
        .def("get_role", &cnf_grammar::RoleRestrictionImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_concept", &cnf_grammar::RoleRestrictionImpl::get_concept, nb::rv_policy::reference_internal);
    nb::class_<cnf_grammar::RoleIdentityImpl, cnf_grammar::IConstructor<RoleTag>>(m, "CNFRoleIdentity")
        .def("get_concept", &cnf_grammar::RoleIdentityImpl::get_concept, nb::rv_policy::reference_internal);

    cnf_grammar::bind_boolean_atomic_state<formalism::StaticTag>(m, "CNFBooleanStaticAtomicState");
    cnf_grammar::bind_boolean_atomic_state<formalism::FluentTag>(m, "CNFBooleanFluentAtomicState");
    cnf_grammar::bind_boolean_atomic_state<formalism::DerivedTag>(m, "CNFBooleanDerivedAtomicState");
    cnf_grammar::bind_boolean_nonempty<ConceptTag>(m, "CNFBooleanConceptNonempty");
    cnf_grammar::bind_boolean_nonempty<RoleTag>(m, "CNFBooleanRoleNonempty");

    cnf_grammar::bind_numerical_count<ConceptTag>(m, "CNFNumericalConceptCount");
    cnf_grammar::bind_numerical_count<RoleTag>(m, "CNFNumericalRoleCount");
    nb::class_<cnf_grammar::NumericalDistanceImpl, cnf_grammar::IConstructor<NumericalTag>>(m, "CNFNumericalDistance")
        .def("get_left_concept", &cnf_grammar::NumericalDistanceImpl::get_left_concept, nb::rv_policy::reference_internal)
        .def("get_role", &cnf_grammar::NumericalDistanceImpl::get_role, nb::rv_policy::reference_internal)
        .def("get_right_concept", &cnf_grammar::NumericalDistanceImpl::get_right_concept, nb::rv_policy::reference_internal);

    nb::class_<cnf_grammar::Grammar>(m, "CNFGrammar")
        .def(nb::init<const std::string&, formalism::Domain>(), "bnf_description"_a, "domain"_a)
        .def_static("create", &cnf_grammar::Grammar::create, "type"_a, "domain"_a)
        .def("__str__", [](const cnf_grammar::Grammar& self) { return to_string(self); })
        .def("accept", &cnf_grammar::Grammar::accept, "visitor"_a)
        .def("test_match", [](const cnf_grammar::Grammar& self, Constructor<ConceptTag> constructor) { return self.test_match(constructor); })
        .def("test_match", [](const cnf_grammar::Grammar& self, Constructor<RoleTag> constructor) { return self.test_match(constructor); })
        .def("test_match", [](const cnf_grammar::Grammar& self, Constructor<BooleanTag> constructor) { return self.test_match(constructor); })
        .def("test_match", [](const cnf_grammar::Grammar& self, Constructor<NumericalTag> constructor) { return self.test_match(constructor); })

        .def("get_repositories", &cnf_grammar::Grammar::get_repositories, nb::rv_policy::reference_internal)
        .def("get_domain", &cnf_grammar::Grammar::get_domain, nb::rv_policy::reference_internal)
        .def("get_concept_start_symbol", &cnf_grammar::Grammar::get_start_symbol<ConceptTag>, nb::rv_policy::reference_internal)
        .def("get_role_start_symbol", &cnf_grammar::Grammar::get_start_symbol<RoleTag>, nb::rv_policy::reference_internal)
        .def("get_boolean_start_symbol", &cnf_grammar::Grammar::get_start_symbol<BooleanTag>, nb::rv_policy::reference_internal)
        .def("get_numerical_start_symbol", &cnf_grammar::Grammar::get_start_symbol<NumericalTag>, nb::rv_policy::reference_internal)
        .def("get_concept_derivation_rules", &cnf_grammar::Grammar::get_derivation_rules<ConceptTag>, nb::rv_policy::reference_internal)
        .def("get_role_derivation_rules", &cnf_grammar::Grammar::get_derivation_rules<RoleTag>, nb::rv_policy::reference_internal)
        .def("get_boolean_derivation_rules", &cnf_grammar::Grammar::get_derivation_rules<BooleanTag>, nb::rv_policy::reference_internal)
        .def("get_numerical_derivation_rules", &cnf_grammar::Grammar::get_derivation_rules<NumericalTag>, nb::rv_policy::reference_internal)
        .def("get_concept_substitution_rules", &cnf_grammar::Grammar::get_substitution_rules<ConceptTag>, nb::rv_policy::reference_internal)
        .def("get_role_substitution_rules", &cnf_grammar::Grammar::get_substitution_rules<RoleTag>, nb::rv_policy::reference_internal)
        .def("get_boolean_substitution_rules", &cnf_grammar::Grammar::get_substitution_rules<BooleanTag>, nb::rv_policy::reference_internal)
        .def("get_numerical_substitution_rules", &cnf_grammar::Grammar::get_substitution_rules<NumericalTag>, nb::rv_policy::reference_internal);

    nb::class_<cnf_grammar::Repositories>(m, "CNFRepositories")
        .def(nb::init<>())
        .def("get_or_create_concept_bot", &cnf_grammar::Repositories::get_or_create_concept_bot, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_top", &cnf_grammar::Repositories::get_or_create_concept_top, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_intersection",
             &cnf_grammar::Repositories::get_or_create_concept_intersection,
             "left_concept"_a,
             "right_concept"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_union",
             &cnf_grammar::Repositories::get_or_create_concept_union,
             "left_concept"_a,
             "right_concept"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_negation", &cnf_grammar::Repositories::get_or_create_concept_negation, "concept_"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_value_restriction",
             &cnf_grammar::Repositories::get_or_create_concept_value_restriction,
             "role"_a,
             "concept_"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_existential_quantification",
             &cnf_grammar::Repositories::get_or_create_concept_existential_quantification,
             "role"_a,
             "concept_"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_role_value_map_containment",
             &cnf_grammar::Repositories::get_or_create_concept_role_value_map_containment,
             "left_role"_a,
             "right_role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_role_value_map_equality",
             &cnf_grammar::Repositories::get_or_create_concept_role_value_map_equality,
             "left_role"_a,
             "right_role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_nominal", &cnf_grammar::Repositories::get_or_create_concept_nominal, "object"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_universal", &cnf_grammar::Repositories::get_or_create_role_universal, nb::rv_policy::reference_internal)
        .def("get_or_create_role_intersection",
             &cnf_grammar::Repositories::get_or_create_role_intersection,
             "left_role"_a,
             "right_role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_union", &cnf_grammar::Repositories::get_or_create_role_union, "left_role"_a, "right_role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_complement", &cnf_grammar::Repositories::get_or_create_role_complement, "role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_inverse", &cnf_grammar::Repositories::get_or_create_role_inverse, "role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_role_composition",
             &cnf_grammar::Repositories::get_or_create_role_composition,
             "left_role"_a,
             "right_role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_transitive_closure",
             &cnf_grammar::Repositories::get_or_create_role_transitive_closure,
             "role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_reflexive_transitive_closure",
             &cnf_grammar::Repositories::get_or_create_role_reflexive_transitive_closure,
             "role"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_restriction",
             &cnf_grammar::Repositories::get_or_create_role_restriction,
             "role"_a,
             "concept_"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_identity", &cnf_grammar::Repositories::get_or_create_role_identity, "concept_"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_state_static",
             &cnf_grammar::Repositories::get_or_create_concept_atomic_state<StaticTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_state_fluent",
             &cnf_grammar::Repositories::get_or_create_concept_atomic_state<FluentTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_state_derived",
             &cnf_grammar::Repositories::get_or_create_concept_atomic_state<DerivedTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_goal_static",
             &cnf_grammar::Repositories::get_or_create_concept_atomic_goal<StaticTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_goal_fluent",
             &cnf_grammar::Repositories::get_or_create_concept_atomic_goal<FluentTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_atomic_goal_derived",
             &cnf_grammar::Repositories::get_or_create_concept_atomic_goal<DerivedTag>,
             "predicate"_a,
             "polarity"_a)
        .def("get_or_create_role_atomic_state_static",
             &cnf_grammar::Repositories::get_or_create_role_atomic_state<StaticTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_state_fluent",
             &cnf_grammar::Repositories::get_or_create_role_atomic_state<FluentTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_state_derived",
             &cnf_grammar::Repositories::get_or_create_role_atomic_state<DerivedTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_goal_static",
             &cnf_grammar::Repositories::get_or_create_role_atomic_goal<StaticTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_goal_fluent",
             &cnf_grammar::Repositories::get_or_create_role_atomic_goal<FluentTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_atomic_goal_derived",
             &cnf_grammar::Repositories::get_or_create_role_atomic_goal<DerivedTag>,
             "predicate"_a,
             "polarity"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_atomic_state_static",
             &cnf_grammar::Repositories::get_or_create_boolean_atomic_state<StaticTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_atomic_state_fluent",
             &cnf_grammar::Repositories::get_or_create_boolean_atomic_state<FluentTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_atomic_state_derived",
             &cnf_grammar::Repositories::get_or_create_boolean_atomic_state<DerivedTag>,
             "predicate"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_nonempty_concept",
             &cnf_grammar::Repositories::get_or_create_boolean_nonempty<ConceptTag>,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_nonempty_role",
             &cnf_grammar::Repositories::get_or_create_boolean_nonempty<RoleTag>,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_count_concept",
             &cnf_grammar::Repositories::get_or_create_numerical_count<ConceptTag>,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_count_role",
             &cnf_grammar::Repositories::get_or_create_numerical_count<RoleTag>,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_distance",
             &cnf_grammar::Repositories::get_or_create_numerical_distance,
             "left_concept"_a,
             "role"_a,
             "right_concept"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_nonterminal",
             &cnf_grammar::Repositories::get_or_create_nonterminal<ConceptTag>,
             "name"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_nontermina", &cnf_grammar::Repositories::get_or_create_nonterminal<RoleTag>, "name"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_nonterminal",
             &cnf_grammar::Repositories::get_or_create_nonterminal<ConceptTag>,
             "name"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_nonterminal", &cnf_grammar::Repositories::get_or_create_nonterminal<RoleTag>, "name"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_concept_derivation_rule",
             &cnf_grammar::Repositories::get_or_create_derivation_rule<ConceptTag>,
             "nonterminal"_a,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_derivation_rule",
             &cnf_grammar::Repositories::get_or_create_derivation_rule<RoleTag>,
             "nonterminal"_a,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_derivation",
             &cnf_grammar::Repositories::get_or_create_derivation_rule<ConceptTag>,
             "nonterminal"_a,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_derivation_rule",
             &cnf_grammar::Repositories::get_or_create_derivation_rule<RoleTag>,
             "nonterminal"_a,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_concept_substitution_rule",
             &cnf_grammar::Repositories::get_or_create_substitution_rule<ConceptTag>,
             "nonterminal"_a,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_role_substitution_rule",
             &cnf_grammar::Repositories::get_or_create_substitution_rule<RoleTag>,
             "nonterminal"_a,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_boolean_substitution_rule",
             &cnf_grammar::Repositories::get_or_create_substitution_rule<ConceptTag>,
             "nonterminal"_a,
             "constructor"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_numerical_substitution_rule",
             &cnf_grammar::Repositories::get_or_create_substitution_rule<RoleTag>,
             "nonterminal"_a,
             "constructor"_a,
             nb::rv_policy::reference_internal);

    /* CNFGrammarVisitor */
    nb::class_<cnf_grammar::IVisitor, cnf_grammar::IPyVisitor>(m, "CNFGrammarVisitor")  //
        .def(nb::init<>())
        .def("visit", nb::overload_cast<cnf_grammar::ConceptBot>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptTop>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptAtomicState<formalism::StaticTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptAtomicState<formalism::FluentTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptAtomicState<formalism::DerivedTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptAtomicGoal<formalism::StaticTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptAtomicGoal<formalism::FluentTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptAtomicGoal<formalism::DerivedTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptIntersection>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptUnion>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptNegation>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptValueRestriction>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptExistentialQuantification>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptRoleValueMapContainment>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptRoleValueMapEquality>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::ConceptNominal>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleUniversal>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleAtomicState<formalism::StaticTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleAtomicState<formalism::FluentTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleAtomicState<formalism::DerivedTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleAtomicGoal<formalism::StaticTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleAtomicGoal<formalism::FluentTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleAtomicGoal<formalism::DerivedTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleIntersection>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleUnion>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleComplement>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleInverse>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleComposition>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleTransitiveClosure>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleReflexiveTransitiveClosure>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleRestriction>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::RoleIdentity>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::BooleanAtomicState<formalism::StaticTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::BooleanAtomicState<formalism::FluentTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::BooleanAtomicState<formalism::DerivedTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::BooleanNonempty<ConceptTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::BooleanNonempty<RoleTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::NumericalCount<ConceptTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::NumericalCount<RoleTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::NumericalDistance>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::NonTerminal<ConceptTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::NonTerminal<RoleTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::NonTerminal<BooleanTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::NonTerminal<NumericalTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::DerivationRule<ConceptTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::DerivationRule<RoleTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::DerivationRule<BooleanTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::DerivationRule<NumericalTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::SubstitutionRule<ConceptTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::SubstitutionRule<RoleTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::SubstitutionRule<BooleanTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<cnf_grammar::SubstitutionRule<NumericalTag>>(&cnf_grammar::IVisitor::visit))
        .def("visit", nb::overload_cast<const cnf_grammar::Grammar&>(&cnf_grammar::IVisitor::visit));

    nb::class_<IRefinementPruningFunction, IPyRefinementPruningFunction>(m, "IRefinementPruningFunction")
        .def(nb::init<>())
        .def("should_prune", nb::overload_cast<Constructor<ConceptTag>>(&IRefinementPruningFunction::should_prune), "concept"_a)
        .def("should_prune", nb::overload_cast<Constructor<RoleTag>>(&IRefinementPruningFunction::should_prune), "role"_a)
        .def("should_prune", nb::overload_cast<Constructor<BooleanTag>>(&IRefinementPruningFunction::should_prune), "boolean"_a)
        .def("should_prune", nb::overload_cast<Constructor<NumericalTag>>(&IRefinementPruningFunction::should_prune), "numerical"_a);

    nb::class_<StateListRefinementPruningFunction, IRefinementPruningFunction>(m, "StateListRefinementPruningFunction")
        .def(nb::init<const mimir::datasets::GeneralizedStateSpace&>())
        .def(nb::init<const mimir::datasets::GeneralizedStateSpace&, const mimir::graphs::ClassGraph&>())
        .def(nb::init<mimir::formalism::ProblemMap<mimir::search::StateList>>());

    nb::class_<cnf_grammar::GeneratedSentencesContainer>(m, "GeneratedSentencesContainer")  //
        .def(nb::init<>())
        .def("get_concepts", nb::overload_cast<>(&cnf_grammar::GeneratedSentencesContainer::get<ConceptTag>, nb::const_), nb::rv_policy::reference_internal)
        .def("get_roles", nb::overload_cast<>(&cnf_grammar::GeneratedSentencesContainer::get<RoleTag>, nb::const_), nb::rv_policy::reference_internal)
        .def("get_booleans", nb::overload_cast<>(&cnf_grammar::GeneratedSentencesContainer::get<BooleanTag>, nb::const_), nb::rv_policy::reference_internal)
        .def("get_numericals",
             nb::overload_cast<>(&cnf_grammar::GeneratedSentencesContainer::get<NumericalTag>, nb::const_),
             nb::rv_policy::reference_internal);

    nb::class_<cnf_grammar::GeneratorVisitor>(m, "GeneratorVisitor")
        .def(nb::init<IRefinementPruningFunction&, cnf_grammar::GeneratedSentencesContainer&, Repositories&, size_t>())
        .def("visit", nb::overload_cast<const cnf_grammar::Grammar&>(&cnf_grammar::GeneratorVisitor::visit));
}
}
