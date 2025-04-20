#include "bindings.hpp"

#include "../../init_declarations.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/trampoline.h>

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::languages::general_policies
{

class IPyVisitor : public IVisitor
{
public:
    NB_TRAMPOLINE(IVisitor, 40);

    /* Trampoline (need one for each virtual function) */
    void visit(PositiveBooleanCondition constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(NegativeBooleanCondition constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(GreaterNumericalCondition constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(EqualNumericalCondition constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(PositiveBooleanEffect constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(NegativeBooleanEffect constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(UnchangedBooleanEffect constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(IncreaseNumericalEffect constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(DecreaseNumericalEffect constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(UnchangedNumericalEffect constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(NamedFeature<dl::ConceptTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(NamedFeature<dl::RoleTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(NamedFeature<dl::NumericalTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(NamedFeature<dl::BooleanTag> constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(Rule constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
    void visit(GeneralPolicy constructor) override { NB_OVERRIDE_PURE(visit, constructor); }
};

void bind_module_definitions(nb::module_& m)
{
    bind_named_feature<dl::ConceptTag>(m, "NamedConcept");
    bind_named_feature<dl::RoleTag>(m, "NamedRole");
    bind_named_feature<dl::BooleanTag>(m, "NamedBoolean");
    bind_named_feature<dl::NumericalTag>(m, "NamedNumerical");

    nb::class_<ICondition>(m, "Condition")  //
        .def("__str__", [](const ICondition& self) { return to_string(self); })
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
        .def("evaluate", &RuleImpl::evaluate, nb::rv_policy::copy, "source_evaluation_context"_a, "target_evaluation_context"_a)
        .def("accept", &RuleImpl::accept, "visitor"_a)
        .def("get_index", &RuleImpl::get_index, nb::rv_policy::copy)
        .def("get_conditions", &RuleImpl::get_conditions, nb::rv_policy::copy)
        .def("get_effects", &RuleImpl::get_effects, nb::rv_policy::copy);
    nb::bind_vector<RuleList>(m, "RuleList");

    nb::class_<GeneralPolicyImpl>(m, "GeneralPolicy")  //
        .def("__str__", [](const GeneralPolicyImpl& self) { return to_string(self); });
}

}
