#include "mimir/common/translations.hpp"

#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/ground_atom.hpp"

namespace mimir
{

void to_literals(SimpleEffectList effect, LiteralList& ref_literals)
{
    for (const auto& simple_effect : effect)
    {
        if (!simple_effect->get_quantified_variables().empty())
        {
            throw std::runtime_error("Universal quantified effects not supported");
        }

        if (!simple_effect->get_conditions().empty())
        {
            throw std::runtime_error("Conditional effects not supported");
        }

        ref_literals.push_back(simple_effect->get_effect());
    }
}

void to_ground_atoms(const ConstBitsetView& bitset, const PDDLFactories& pddl_factories, GroundAtomList& out_ground_atoms)
{
    out_ground_atoms.clear();

    for (const auto& atom_id : bitset)
    {
        out_ground_atoms.emplace_back(pddl_factories.get_ground_atom(atom_id));
    }
}

void to_ground_atoms(ConstView<StateDispatcher<DenseStateTag>> state, const PDDLFactories& pddl_factories, GroundAtomList& out_ground_atoms)
{
    const auto& bitset = state.get_atoms_bitset();
    to_ground_atoms(bitset, pddl_factories, out_ground_atoms);
}

Plan to_plan(const std::vector<ConstView<ActionDispatcher<StateReprTag>>>& action_view_list)
{
    std::vector<std::string> actions;
    auto cost = 0;
    for (const auto action : action_view_list)
    {
        std::stringstream ss;
        ss << action;
        actions.push_back(ss.str());
        cost += action.get_cost();
    }
    return Plan(std::move(actions), cost);
}

}
