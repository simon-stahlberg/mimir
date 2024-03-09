#ifndef MIMIR_COMMON_TRANSLATIONS_HPP_
#define MIMIR_COMMON_TRANSLATIONS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/states/bitset/bitset.hpp"

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

}

#endif  // MIMIR_COMMON_TRANSLATIONS_HPP_
