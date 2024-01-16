#ifndef MIMIR_SEARCH_SUCCESSOR_GENERATOR_BASE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_GENERATOR_BASE_HPP_

#include "state.hpp"

#include "../common/config.hpp"
#include "../common/config_type_trait.hpp"

#include "../formalism/action.hpp"

#include <stdexcept>
#include <vector>


namespace mimir
{

/// @brief Top-level CRTP based interface for a SuccessorGenerator.
/// @tparam Derived
template<typename Derived>
class SuccessorGeneratorBase {
private:
    using Configuration = typename ConfigurationTypeTrait<Derived>::ConfigurationType;

    SuccessorGeneratorBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }

public:
    ActionList generate_applicable_actions(const State<Configuration>& state) {
        return self().generate_applicable_actions_impl(state);
    }
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_GENERATOR_BASE_HPP_
