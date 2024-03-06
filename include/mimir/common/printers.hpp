#ifndef MIMIR_COMMON_PRINTERS_HPP_
#define MIMIR_COMMON_PRINTERS_HPP_

#include "translations.hpp"

#include "../formalism/domain/declarations.hpp"
#include "../search/states/bitset/bitset.hpp"
#include "../formalism/common/types.hpp"

#include <ostream>
#include <functional>
#include <cstdint>
#include <tuple>
#include <vector>


namespace mimir
{

/// @brief Prints a vector to the output stream.
template<typename T>
std::ostream& operator<<(
    std::ostream& os,
    const std::vector<const T*>& vec)
{
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (i != 0)
        {
            os << ", ";
        }
        os << *vec[i];
    }
    os << "]";

    return os;
}

/// @brief Prints a State to the output stream.
template<IsPlanningModeTag P>
std::ostream& operator<<(
    std::ostream& os,
    const std::tuple<
        ConstView<StateDispatcher<BitsetStateTag, P>>,
        const PDDLFactories&>& data)
{
    const auto [state, pddl_factories] = data;
    auto out_ground_atoms = GroundAtomList{};
    to_ground_atoms(state, pddl_factories, out_ground_atoms);

    os << "State("
       << "state_id=" << state.get_id() << ", "
       << "ground_atoms=" << out_ground_atoms
       << ")";

    return os;
}

}

#endif
