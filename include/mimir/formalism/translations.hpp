#ifndef MIMIR_FORMALISM_TRANSLATIONS_HPP_
#define MIMIR_FORMALISM_TRANSLATIONS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/factories.hpp"

#include <tuple>
#include <variant>
#include <vector>

namespace mimir
{

/// @brief Compute the positive normalized problem into a new PDDLFactory.
/// @param problem
/// @return
extern std::tuple<Problem, PDDLFactories> to_positive_normalized_problem(const Problem& problem);

/// @brief Compute the effect normalized problem into a new PDDLFactory.
/// @param problem
/// @return
extern std::tuple<Problem, PDDLFactories> to_effect_normalized_problem(const Problem& problem);

/// @brief Compute the delete relaxed problem where all negative effects are removed.
/// @param problem
/// @return
extern std::tuple<Problem, PDDLFactories> to_delete_relaxed_problem(const Problem& problem);

}

#endif