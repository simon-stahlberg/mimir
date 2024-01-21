#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATOR_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATOR_HPP_

#include "config.hpp"
#include "state.hpp"
#include "state_builder.hpp"
#include "state_view.hpp"
#include "state_view.hpp"
#include "type_traits.hpp"

#include "../buffer/containers/unordered_set.hpp"
#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"

#include <unordered_set>


namespace mimir
{

/**
 * Interface class
*/
template<typename Derived>
class SuccessorStateGeneratorBase : public UncopyableMixin<SuccessorStateGeneratorBase<Derived>> {
private:
    using C = typename TypeTraits<Derived>::ConfigType;

    SuccessorStateGeneratorBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

protected:
    UnorderedSet<State<C>> m_states;

    Builder<State<C>> m_state_builder;

public:
    [[nodiscard]] View<State<C>> get_or_create_initial_state(Problem problem) {
        return self().get_or_create_initial_state_impl(problem);
    }

    [[nodiscard]] View<State<C>> get_or_create_successor_state(View<State<C>> state, GroundAction action) {
        return self().get_or_create_successor_state_impl(state, action);
    }
};


/**
 * Implementation class (general)
*/
template<Config C>
class SuccessorStateGenerator : public SuccessorStateGeneratorBase<SuccessorStateGenerator<C>> { };


/**
 * Type traits
*/
template<Config C>
struct TypeTraits<SuccessorStateGenerator<C>> {
    using ConfigType = C;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATOR_HPP_