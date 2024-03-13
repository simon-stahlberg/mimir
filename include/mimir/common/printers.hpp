#ifndef MIMIR_COMMON_PRINTERS_HPP_
#define MIMIR_COMMON_PRINTERS_HPP_

#include "mimir/common/translations.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/states.hpp"

#include <ostream>
#include <tuple>
#include <vector>

namespace mimir
{

/// @brief Prints a State to the output stream.
std::ostream& operator<<(std::ostream& os, const std::tuple<ConstView<StateDispatcher<DenseStateTag>>, const PDDLFactories&>& data);

/// @brief Prints an Action to the output stream.
std::ostream& operator<<(std::ostream& os, const std::tuple<ConstView<ActionDispatcher<DenseStateTag>>, const PDDLFactories&>& data);

std::ostream& operator<<(std::ostream& os, const ConstDenseActionViewProxy& action);

}

#endif
