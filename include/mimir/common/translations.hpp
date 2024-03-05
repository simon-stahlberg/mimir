#ifndef MIMIR_COMMON_TRANSLATIONS_HPP_
#define MIMIR_COMMON_TRANSLATIONS_HPP_

#include "../formalism/domain/declarations.hpp"

namespace mimir
{

/// @brief Translates a condition into a conjunction of literals if possible, otherwise throws an error.
void to_literals(Condition precondition, LiteralList& out_literals);

/// @brief Translates an effect into a list of literals if possible, otherwise throws an error.
void to_literals(Effect effect, LiteralList& out_literals);

}

#endif  // MIMIR_COMMON_TRANSLATIONS_HPP_
