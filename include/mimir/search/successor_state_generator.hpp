#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATOR_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATOR_HPP_

#include "config.hpp"
#include "grounded/state_view.hpp"
#include "grounded/state_builder.hpp"
#include "lifted/state_view.hpp"
#include "lifted/state_builder.hpp"
#include "type_traits.hpp"

#include "../buffer/containers/unordered_set.hpp"
#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"


namespace mimir
{

/**
 * Interface class
*/
template<typename Derived>
class SuccessorStateGeneratorBase : public UncopyableMixin<SuccessorStateGeneratorBase<Derived>> {
private:
    using C = typename TypeTraits<Derived>::ConfigTagType;

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
 * Implementation class.
 *
 * We provide specialized implementations for
 * - Grounded in grounded/successor_state_generator.hpp
 * - Lifted in lifted/successor_state_generator.hpp
*/
template<IsConfig C>
class SuccessorStateGenerator : public SuccessorStateGeneratorBase<SuccessorStateGenerator<C>> { };


/**
 * Type traits
*/
template<IsConfig C>
struct TypeTraits<SuccessorStateGenerator<C>> {
    using ConfigTagType = C;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATOR_HPP_
