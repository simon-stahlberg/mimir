#include "bindings.hpp"

#include "../../init_declarations.hpp"

#include <nanobind/trampoline.h>

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::languages::general_policies
{

void bind_module_definitions(nb::module_& m)
{
    nb::enum_<SolvabilityStatus>(m, "SolvabilityStatus")  //
        .value("SOLVED", SolvabilityStatus::SOLVED)
        .value("CYCLIC", SolvabilityStatus::CYCLIC)
        .value("UNSOLVABLE", SolvabilityStatus::UNSOLVABLE);

    nb::class_<GeneralPoliciesRefinementPruningFunction, dl::IRefinementPruningFunction>(m, "GeneralPoliciesPruningFunction")
        .def(nb::init<const mimir::datasets::GeneralizedStateSpace&, dl::DenotationRepositories&>(),
             "generalized_state_space"_a,
             "ref_denotation_repositories"_a)
        .def(nb::init<const mimir::datasets::GeneralizedStateSpace&, const mimir::graphs::ClassGraph&, dl::DenotationRepositories&>(),
             "generalized_state_space"_a,
             "class_graph"_a,
             "ref_denotation_repositories"_a)
        .def(nb::init<mimir::search::StateProblemList, mimir::search::StateProblemPairList, dl::DenotationRepositories&>(),
             "states"_a,
             "transitions"_a,
             "ref_denotation_repositories"_a);

    bind_named_feature<dl::ConceptTag>(m, "NamedConcept");
    bind_named_feature<dl::RoleTag>(m, "NamedRole");
    bind_named_feature<dl::BooleanTag>(m, "NamedBoolean");
    bind_named_feature<dl::NumericalTag>(m, "NamedNumerical");

    nb::class_<ICondition>(m, "Condition")  //
        .def("__str__", [](const ICondition& self) { return to_string(self); })
        .def("__eq__", [](const ICondition& lhs, const ICondition& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const ICondition& lhs, const ICondition& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const ICondition& self) { return std::hash<Condition> {}(&self); })
        .def("evaluate", &ICondition::evaluate, nb::rv_policy::copy, "evaluation_context"_a)
        .def("accept", &ICondition::accept, "visitor"_a)
        .def("get_index", &ICondition::get_index, nb::rv_policy::copy);
    nb::bind_vector<ConditionList>(m, "ConditionList");

    nb::class_<PositiveBooleanConditionImpl, ICondition>(m, "PositiveBooleanCondition")
        .def("get_feature", &PositiveBooleanConditionImpl::get_feature, nb::rv_policy::reference_internal);

    nb::class_<NegativeBooleanConditionImpl, ICondition>(m, "NegativeBooleanCondition")
        .def("get_feature", &NegativeBooleanConditionImpl::get_feature, nb::rv_policy::reference_internal);

    nb::class_<GreaterNumericalConditionImpl, ICondition>(m, "GreaterNumericalCondition")
        .def("get_feature", &GreaterNumericalConditionImpl::get_feature, nb::rv_policy::reference_internal);

    nb::class_<EqualNumericalConditionImpl, ICondition>(m, "EqualNumericalCondition")
        .def("get_feature", &EqualNumericalConditionImpl::get_feature, nb::rv_policy::reference_internal);

    nb::class_<IEffect>(m, "Effect")  //
        .def("__str__", [](const IEffect& self) { return to_string(self); })
        .def("__eq__", [](const IEffect& lhs, const IEffect& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const IEffect& lhs, const IEffect& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const IEffect& self) { return std::hash<Effect> {}(&self); })
        .def("evaluate", &IEffect::evaluate, nb::rv_policy::copy, "source_evaluation_context"_a, "target_evaluation_context"_a)
        .def("accept", &IEffect::accept, "visitor"_a)
        .def("get_index", &IEffect::get_index, nb::rv_policy::copy);
    nb::bind_vector<EffectList>(m, "EffectList");

    nb::class_<PositiveBooleanEffectImpl, IEffect>(m, "PositiveBooleanEffect")
        .def("get_feature", &PositiveBooleanEffectImpl::get_feature, nb::rv_policy::reference_internal);

    nb::class_<NegativeBooleanEffectImpl, IEffect>(m, "NegativeBooleanEffect")
        .def("get_feature", &NegativeBooleanEffectImpl::get_feature, nb::rv_policy::reference_internal);

    nb::class_<UnchangedBooleanEffectImpl, IEffect>(m, "UnchangedBooleanEffect")
        .def("get_feature", &UnchangedBooleanEffectImpl::get_feature, nb::rv_policy::reference_internal);

    nb::class_<IncreaseNumericalEffectImpl, IEffect>(m, "IncreaseNumericalEffect")
        .def("get_feature", &IncreaseNumericalEffectImpl::get_feature, nb::rv_policy::reference_internal);

    nb::class_<DecreaseNumericalEffectImpl, IEffect>(m, "DecreaseNumericalEffect")
        .def("get_feature", &DecreaseNumericalEffectImpl::get_feature, nb::rv_policy::reference_internal);

    nb::class_<UnchangedNumericalEffectImpl, IEffect>(m, "UnchangedNumericalEffect")
        .def("get_feature", &UnchangedNumericalEffectImpl::get_feature, nb::rv_policy::reference_internal);

    nb::class_<RuleImpl>(m, "Rule")  //
        .def("__str__", [](const RuleImpl& self) { return to_string(self); })
        .def("__eq__", [](const RuleImpl& lhs, const RuleImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const RuleImpl& lhs, const RuleImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const RuleImpl& self) { return std::hash<Rule> {}(&self); })
        .def("evaluate", &RuleImpl::evaluate, nb::rv_policy::copy, "source_evaluation_context"_a, "target_evaluation_context"_a)
        .def("accept", &RuleImpl::accept, "visitor"_a)
        .def("get_index", &RuleImpl::get_index, nb::rv_policy::copy)
        .def("get_conditions", &RuleImpl::get_conditions, nb::rv_policy::copy)
        .def("get_effects", &RuleImpl::get_effects, nb::rv_policy::copy);
    nb::bind_vector<RuleList>(m, "RuleList");

    nb::class_<GeneralPolicyImpl>(m, "GeneralPolicy")  //
        .def("__str__", [](const GeneralPolicyImpl& self) { return to_string(self); })
        .def("__eq__", [](const GeneralPolicyImpl& lhs, const GeneralPolicyImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const GeneralPolicyImpl& lhs, const GeneralPolicyImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GeneralPolicyImpl& self) { return std::hash<GeneralPolicy>()(&self); })
        .def("evaluate", &GeneralPolicyImpl::evaluate, "source_context"_a, "target_contest"_a)
        .def("accept", &GeneralPolicyImpl::accept, "visitor"_a)
        .def("is_terminating", nb::overload_cast<Repositories&>(&GeneralPolicyImpl::is_terminating, nb::const_), "repositories"_a)
        .def(
            "solves",
            [](const GeneralPolicyImpl& self, const datasets::StateSpace& state_space, dl::DenotationRepositories& denotation_repositories) -> SolvabilityStatus
            { return self.solves(state_space, denotation_repositories); },
            "state_space"_a,
            "denotation_repositories"_a)
        .def(
            "solves",
            [](const GeneralPolicyImpl& self,
               const datasets::StateSpace& state_space,
               const graphs::VertexIndexList& vertices,
               dl::DenotationRepositories& denotation_repositories) -> SolvabilityStatus
            { return self.solves(state_space, vertices, denotation_repositories); },
            "state_space"_a,
            "vertex_indices"_a,
            "denotation_repositories"_a)
        .def(
            "solves",
            [](const GeneralPolicyImpl& self,
               const datasets::GeneralizedStateSpace& state_space,
               dl::DenotationRepositories& denotation_repositories) -> SolvabilityStatus { return self.solves(state_space, denotation_repositories); },
            "state_space"_a,
            "denotation_repositories"_a)
        .def(
            "solves",
            [](const GeneralPolicyImpl& self,
               const datasets::GeneralizedStateSpace& state_space,
               const graphs::VertexIndexList& vertices,
               dl::DenotationRepositories& denotation_repositories) -> SolvabilityStatus
            { return self.solves(state_space, vertices, denotation_repositories); },
            "state_space"_a,
            "vertex_indices"_a,
            "denotation_repositories"_a)
        .def("find_solution", &GeneralPolicyImpl::find_solution, "search_context"_a)
        .def("get_index", &GeneralPolicyImpl::get_index)
        .def("get_boolean_features", &GeneralPolicyImpl::get_features<dl::BooleanTag>, nb::rv_policy::copy)
        .def("get_numerical_features", &GeneralPolicyImpl::get_features<dl::NumericalTag>, nb::rv_policy::copy)
        .def("get_rules", &GeneralPolicyImpl::get_rules, nb::rv_policy::copy);

    nb::class_<Repositories>(m, "Repositories")
        .def(nb::init<>())
        .def("get_or_create_named_feature",
             &Repositories::get_or_create_named_feature<dl::ConceptTag>,
             "name"_a,
             "concept"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_named_feature", &Repositories::get_or_create_named_feature<dl::RoleTag>, "name"_a, "role"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_named_feature",
             &Repositories::get_or_create_named_feature<dl::BooleanTag>,
             "name"_a,
             "boolean"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_named_feature",
             &Repositories::get_or_create_named_feature<dl::NumericalTag>,
             "name"_a,
             "numerical"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_positive_boolean_condition",
             &Repositories::get_or_create_positive_boolean_condition,
             "named_boolean",
             nb::rv_policy::reference_internal)
        .def("get_or_create_negative_boolean_condition",
             &Repositories::get_or_create_negative_boolean_condition,
             "named_boolean",
             nb::rv_policy::reference_internal)
        .def("get_or_create_greater_numerical_condition",
             &Repositories::get_or_create_greater_numerical_condition,
             "named_numerical",
             nb::rv_policy::reference_internal)
        .def("get_or_create_equal_numerical_condition",
             &Repositories::get_or_create_equal_numerical_condition,
             "named_numerical",
             nb::rv_policy::reference_internal)
        .def("get_or_create_positive_boolean_effect", &Repositories::get_or_create_positive_boolean_effect, "named_boolean", nb::rv_policy::reference_internal)
        .def("get_or_create_negative_boolean_effect", &Repositories::get_or_create_negative_boolean_effect, "named_boolean", nb::rv_policy::reference_internal)
        .def("get_or_create_unchanged_boolean_effect",
             &Repositories::get_or_create_unchanged_boolean_effect,
             "named_boolean",
             nb::rv_policy::reference_internal)
        .def("get_or_create_increase_numerical_effect",
             &Repositories::get_or_create_increase_numerical_effect,
             "named_numerical",
             nb::rv_policy::reference_internal)
        .def("get_or_create_decrease_numerical_effect",
             &Repositories::get_or_create_decrease_numerical_effect,
             "named_numerical",
             nb::rv_policy::reference_internal)
        .def("get_or_create_unchanged_numerical_effect",
             &Repositories::get_or_create_unchanged_numerical_effect,
             "named_numerical",
             nb::rv_policy::reference_internal)
        .def("get_or_create_rule", &Repositories::get_or_create_rule, "conditions"_a, "effects"_a, nb::rv_policy::reference_internal)
        .def(
            "get_or_create_general_policy",
            [](Repositories& self, NamedFeatureList<dl::BooleanTag> named_booleans, NamedFeatureList<dl::NumericalTag> named_numericals, RuleList rules)
                -> GeneralPolicy
            {
                return self.get_or_create_general_policy(
                    boost::hana::make_map(boost::hana::make_pair(boost::hana::type_c<dl::BooleanTag>, std::move(named_booleans)),
                                          boost::hana::make_pair(boost::hana::type_c<dl::NumericalTag>, std::move(named_numericals))),
                    std::move(rules));
            },
            "boolean_features"_a,
            "numerical_features"_a,
            "rules"_a,
            nb::rv_policy::reference_internal)
        .def("get_or_create_general_policy",
             nb::overload_cast<const std::string&, const formalism::DomainImpl&, dl::Repositories&>(&Repositories::get_or_create_general_policy),
             "description"_a,
             "domain"_a,
             "dl_repositories"_a,
             nb::rv_policy::reference_internal);

    nb::class_<GeneralPolicyFactory>(m, "GeneralPolicyFactory")
        .def_static("get_or_create_general_policy_gripper",
                    &GeneralPolicyFactory::get_or_create_general_policy_gripper,
                    "domain"_a,
                    "repositories"_a,
                    "dl_repositories"_a,
                    nb::rv_policy::reference)
        .def_static("get_or_create_general_policy_blocks3ops",
                    &GeneralPolicyFactory::get_or_create_general_policy_blocks3ops,
                    "domain"_a,
                    "repositories"_a,
                    "dl_repositories"_a,
                    nb::rv_policy::reference)
        .def_static("get_or_create_general_policy_spanner",
                    &GeneralPolicyFactory::get_or_create_general_policy_spanner,
                    "domain"_a,
                    "repositories"_a,
                    "dl_repositories"_a,
                    nb::rv_policy::reference)
        .def_static("get_or_create_general_policy_delivery",
                    &GeneralPolicyFactory::get_or_create_general_policy_delivery,
                    "domain"_a,
                    "repositories"_a,
                    "dl_repositories"_a,
                    nb::rv_policy::reference);
}

}
