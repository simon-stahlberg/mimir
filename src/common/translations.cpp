#include "mimir/common/translations.hpp"

#include "mimir/formalism/conditions.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/factories.hpp"

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

void to_ground_atoms(const ConstBitsetView& bitset, const PDDLFactories& pddl_factories, GroundAtomList& out_ground_atoms)
{
    out_ground_atoms.clear();

    for (const auto& atom_id : bitset)
    {
        out_ground_atoms.emplace_back(pddl_factories.ground_atoms.get(atom_id));
    }
}

void to_ground_atoms(ConstView<StateDispatcher<DenseStateTag>> state, const PDDLFactories& pddl_factories, GroundAtomList& out_ground_atoms)
{
    const auto& bitset = state.get_atoms_bitset();
    to_ground_atoms(bitset, pddl_factories, out_ground_atoms);
}

}
