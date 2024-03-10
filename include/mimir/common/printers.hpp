#ifndef MIMIR_COMMON_PRINTERS_HPP_
#define MIMIR_COMMON_PRINTERS_HPP_

#include "mimir/common/translations.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions/bitset.hpp"
#include "mimir/search/actions/interface.hpp"
#include "mimir/search/planning_mode.hpp"
#include "mimir/search/states/bitset/bitset.hpp"

#include <ostream>
#include <tuple>
#include <vector>

namespace mimir
{

/// @brief Prints a vector to the output stream.
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<const T*>& vec)
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
std::ostream& operator<<(std::ostream& os, const std::tuple<ConstView<StateDispatcher<BitsetStateTag, P>>, const PDDLFactories&>& data)
{
    const auto [state, pddl_factories] = data;
    auto out_ground_atoms = GroundAtomList {};
    to_ground_atoms(state, pddl_factories, out_ground_atoms);

    os << "State("
       << "state_id=" << state.get_id() << ", "
       << "ground_atoms=" << out_ground_atoms << ")";

    return os;
}

/// @brief Prints an Action to the output stream.
template<IsPlanningModeTag P>
std::ostream& operator<<(std::ostream& os, const std::tuple<ConstView<ActionDispatcher<P, BitsetStateTag>>, const PDDLFactories&>& data)
{
    const auto [action, pddl_factories] = data;

    auto positive_precondition_bitset = action.get_applicability_positive_precondition_bitset();
    auto negative_precondition_bitset = action.get_applicability_negative_precondition_bitset();
    auto positive_effect_bitset = action.get_unconditional_positive_effect_bitset();
    auto negative_effect_bitset = action.get_unconditional_negative_effect_bitset();

    auto positive_precondition = GroundAtomList {};
    auto negative_precondition = GroundAtomList {};
    auto positive_effect = GroundAtomList {};
    auto negative_effect = GroundAtomList {};

    to_ground_atoms(positive_precondition_bitset, pddl_factories, positive_precondition);
    to_ground_atoms(negative_precondition_bitset, pddl_factories, negative_precondition);
    to_ground_atoms(positive_effect_bitset, pddl_factories, positive_effect);
    to_ground_atoms(negative_effect_bitset, pddl_factories, negative_effect);

    auto precondition = GroundAtomList {};
    precondition.insert(precondition.end(), positive_precondition.begin(), positive_precondition.end());
    precondition.insert(precondition.end(), negative_precondition.begin(), negative_precondition.end());

    os << "Action("
       << "precondition=" << precondition << ", "
       << "delete=" << negative_effect << ", "
       << "add=" << positive_effect << ")";

    return os;
}

template<IsPlanningModeTag P>
std::ostream& operator<<(std::ostream& os, const ConstBitsetActionViewProxy<P>& action)
{
    os << "(" << action.get_action()->get_name();
    for (const auto& obj : action.get_objects())
    {
        os << " " << obj->get_name();
    }
    os << ")";
    return os;
}

}

#endif
