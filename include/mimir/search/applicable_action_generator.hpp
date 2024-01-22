#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATOR_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATOR_HPP_

#include "state_view.hpp"
#include "grounded/state_view.hpp"
#include "lifted/state_view.hpp"
#include "type_traits.hpp"

#include "../formalism/problem/declarations.hpp"


namespace mimir
{

/**
 * Interface class.
*/
template<typename Derived>
class ApplicableActionGeneratorBase : public UncopyableMixin<ApplicableActionGeneratorBase<Derived>> {
private:
    using C = typename TypeTraits<Derived>::Config;

    ApplicableActionGeneratorBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Generate all applicable actions for a given state.
    void generate_applicable_actions(StateView<C> state, GroundActionList& out_applicable_actions) {
        self().generate_applicable_actions_impl(state, out_applicable_actions);
    }
};


/**
 * Implementation class.
*/
template<IsConfig C>
class ApplicableActionGenerator : public ApplicableActionGeneratorBase<ApplicableActionGenerator<C>> {
private:
    // Implement Config independent functionality.
};


/**
 * Type traits.
*/
template<IsConfig C>
struct TypeTraits<ApplicableActionGenerator<C>> {
    using Config = C;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATOR_HPP_
