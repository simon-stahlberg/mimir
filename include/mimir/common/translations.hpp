#ifndef MIMIR_COMMON_TRANSLATIONS_HPP_
#define MIMIR_COMMON_TRANSLATIONS_HPP_

#include "../formalism/domain/declarations.hpp"
#include "../search/states/bitset/bitset.hpp"
#include "../formalism/common/types.hpp"


namespace mimir
{

/// @brief Translates a condition into a conjunction of literals if possible, otherwise throws an error.
void to_literals(Condition precondition, LiteralList& out_literals);

/// @brief Translates an effect into a list of literals if possible, otherwise throws an error.
void to_literals(Effect effect, LiteralList& out_literals);

/// @brief Translates a state into a list of ground atoms
template<IsPlanningModeTag P>
void to_ground_atoms(
    ConstView<StateDispatcher<BitsetStateTag, P>> state,
    const PDDLFactories& pddl_factories,
    GroundAtomList& out_ground_atoms)
{
    out_ground_atoms.clear();
    for (const auto& atom_id : state.get_atoms_bitset()) {
        out_ground_atoms.push_back(pddl_factories.ground_atoms.get(atom_id));
    }
}

}

#endif  // MIMIR_COMMON_TRANSLATIONS_HPP_
