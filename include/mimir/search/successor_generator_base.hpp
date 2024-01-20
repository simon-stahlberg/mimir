#ifndef MIMIR_SEARCH_SUCCESSOR_GENERATOR_BASE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_GENERATOR_BASE_HPP_

#include "state.hpp"
#include "state_builder.hpp"
#include "state_view.hpp"
#include "state_builder.hpp"
#include "type_traits.hpp"

#include "../formalism/problem/declarations.hpp"


namespace mimir
{

/// @brief Top-level CRTP based interface for a SuccessorGenerator.
/// @tparam Derived
template<typename Derived>
class SuccessorGeneratorBase : public UncopyableMixin<SuccessorGeneratorBase<Derived>> {
private:
    using C = typename TypeTraits<Derived>::ConfigType;

    SuccessorGeneratorBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Generate all applicable actions for a given state.
    // TODO: This function should return a view to a GroundActionList at some point.
    // The user must assume that GroundActionList becomes
    // invalidated when calling generate_application_actions again.
    GroundActionList generate_applicable_actions(View<State<C>> state) {
        return self().generate_applicable_actions_impl(state);
    }
};


/// @brief A concrete successor generator.
template<Config C>
class SuccessorGenerator : public SuccessorGeneratorBase<SuccessorGenerator<C>> {
private:
    // Implement Config independent functionality.
};


template<Config C>
struct TypeTraits<SuccessorGenerator<C>> {
    using ConfigType = C;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_GENERATOR_BASE_HPP_
