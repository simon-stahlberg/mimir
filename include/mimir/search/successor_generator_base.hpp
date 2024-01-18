#ifndef MIMIR_SEARCH_SUCCESSOR_GENERATOR_BASE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_GENERATOR_BASE_HPP_

#include "state_base.hpp"
#include "state_builder_base.hpp"
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
    void generate_applicable_actions(const State<C>& state, GroundActionList& out_applicable_actions) {
        return self().generate_applicable_actions_impl(state, out_applicable_actions);
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
