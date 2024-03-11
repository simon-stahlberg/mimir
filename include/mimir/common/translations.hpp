#ifndef MIMIR_COMMON_TRANSLATIONS_HPP_
#define MIMIR_COMMON_TRANSLATIONS_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/states.hpp"

namespace mimir
{

/// @brief Translates a condition into a conjunction of literals if possible, otherwise throws an error.
void to_literals(Condition precondition, LiteralList& out_literals);

/// @brief Translates an effect into a list of literals if possible, otherwise throws an error.
void to_literals(Effect effect, LiteralList& out_literals);

/// @brief Translates a bitset into a list of ground atoms
void to_ground_atoms(const ConstBitsetView& bitset, const PDDLFactories& pddl_factories, GroundAtomList& out_ground_atoms);

/// @brief Translates a state into a list of ground atoms
template<IsPlanningModeTag P>
void to_ground_atoms(ConstView<StateDispatcher<BitsetStateTag, P>> state, const PDDLFactories& pddl_factories, GroundAtomList& out_ground_atoms)
{
    const auto& bitset = state.get_atoms_bitset();
    to_ground_atoms(bitset, pddl_factories, out_ground_atoms);
}

/// @brief Translates a ground action list to a plan
template<IsActionDispatcher A>
Plan to_plan(const std::vector<ConstView<A>>& action_view_list)
{
    std::vector<std::string> actions;
    const auto cost = action_view_list.size();
    for (size_t i = 0; i < action_view_list.size(); ++i)
    {
        std::stringstream ss;
        ss << action_view_list[i];
        actions.push_back(ss.str());
        // TODO accumulate the action costs instead of returning the plan length.
    }
    return Plan(std::move(actions), cost);
}

}

#endif  // MIMIR_COMMON_TRANSLATIONS_HPP_
