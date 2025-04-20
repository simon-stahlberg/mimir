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

    nb::class_<ICondition>(m, "Condition");
    nb::bind_vector<ConditionList>(m, "ConditionList");

    nb::class_<PositiveBooleanConditionImpl, ICondition>(m, "PositiveBooleanCondition");

    nb::class_<NegativeBooleanConditionImpl, ICondition>(m, "NegativeBooleanCondition");

    nb::class_<GreaterNumericalConditionImpl, ICondition>(m, "GreaterNumericalCondition");

    nb::class_<EqualNumericalConditionImpl, ICondition>(m, "EqualNumericalCondition");

    nb::class_<IEffect>(m, "Effect");
    nb::bind_vector<EffectList>(m, "EffectList");

    nb::class_<PositiveBooleanEffectImpl, IEffect>(m, "PositiveBooleanEffect");

    nb::class_<NegativeBooleanEffectImpl, IEffect>(m, "NegativeBooleanEffect");

    nb::class_<UnchangedBooleanEffectImpl, IEffect>(m, "UnchangedBooleanEffect");

    nb::class_<IncreaseNumericalEffectImpl, IEffect>(m, "IncreaseNumericalEffect");

    nb::class_<DecreaseNumericalEffectImpl, IEffect>(m, "DecreaseNumericalEffect");

    nb::class_<UnchangedNumericalEffectImpl, IEffect>(m, "UnchangedNumericalEffect");

    nb::class_<RuleImpl>(m, "Rule");
    nb::bind_vector<RuleList>(m, "RuleList");

    nb::class_<GeneralPolicyImpl>(m, "GeneralPolicy");
}

}
