#ifndef MIMIR_COMMON_TRANSLATIONS_HPP_
#define MIMIR_COMMON_TRANSLATIONS_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/compile_flags.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/states.hpp"

namespace mimir
{

/// @brief Translates an effect into a list of literals if possible, otherwise throws an error.
extern void to_literals(EffectList effects, LiteralList& out_literals);

/// @brief Translates a bitset into a list of ground atoms
extern void to_ground_atoms(const ConstBitsetView& bitset, const PDDLFactories& pddl_factories, GroundAtomList& out_ground_atoms);

/// @brief Translates a state into a list of ground atoms
extern void to_ground_atoms(ConstView<StateDispatcher<DenseStateTag>> state, const PDDLFactories& pddl_factories, GroundAtomList& out_ground_atoms);

/// @brief Translates a ground action list to a plan
extern Plan to_plan(const std::vector<ConstView<ActionDispatcher<StateReprTag>>>& action_view_list);

}

#endif  // MIMIR_COMMON_TRANSLATIONS_HPP_
