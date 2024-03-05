#include <mimir/common/translations.hpp>
#include <mimir/formalism/domain/conditions.hpp>
#include <mimir/formalism/domain/effects.hpp>

namespace mimir
{

void to_literals(Condition precondition, LiteralList& ref_literals)
{
    if (const auto* precondition_literal = std::get_if<ConditionLiteralImpl>(precondition))
    {
        ref_literals.emplace_back(precondition_literal->get_literal());
    }
    else if (const auto* precondition_and = std::get_if<ConditionAndImpl>(precondition))
    {
        for (const auto& inner_precondition : precondition_and->get_conditions())
        {
            to_literals(inner_precondition, ref_literals);
        }
    }
    else
    {
        throw std::runtime_error("only conjunctions are supported");
    }
}

void to_literals(Effect effect, LiteralList& ref_literals)
{
    if (const auto* effect_literal = std::get_if<EffectLiteralImpl>(effect))
    {
        ref_literals.emplace_back(effect_literal->get_literal());
    }
    else if (const auto* effect_and = std::get_if<EffectAndImpl>(effect))
    {
        for (const auto& inner_effect : effect_and->get_effects())
        {
            to_literals(inner_effect, ref_literals);
        }
    }
    else
    {
        throw std::runtime_error("only conjunctions are supported");
    }
}

}
