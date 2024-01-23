#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATOR_TEMPLATE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATOR_TEMPLATE_HPP_

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
    using P = typename TypeTraits<Derived>::PlanningMode;

    SuccessorStateGeneratorBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

protected:
    UnorderedSet<State<P>> m_states;

    Builder<State<P>> m_state_builder;

public:
    [[nodiscard]] View<State<P>> get_or_create_initial_state(Problem problem) {
        return self().get_or_create_initial_state_impl(problem);
    }

    [[nodiscard]] View<State<P>> get_or_create_successor_state(View<State<P>> state, GroundAction action) {
        return self().get_or_create_successor_state_impl(state, action);
    }
};

/**
 * ID class.
 *
 * Derive from it to provide your own implementation of a successor state generator.
*/
struct SuccessorStateGeneratorBaseTag {};

template<typename DerivedTag>
concept IsSuccessorStateGeneratorTag = std::derived_from<DerivedTag, SuccessorStateGeneratorBaseTag>;


/**
 * Make SuccessorStateGenerator accept a PlanningModeTag.
 * This makes the template instiation less constraint
 * and we can use the PlanningModeTag of the actual search algorithm.
*/
template<IsSuccessorStateGeneratorTag SG, IsPlanningModeTag P>
struct SuccessorStateGeneratorInstantiation {
    using PlanningModeTag = P;
    using SuccessorStateGeneratorTag = SG;
};

template<typename T>
concept IsSuccessorStateGeneratorInstantiation = requires {
    typename T::PlanningModeTag;
    typename T::SuccessorStateGeneratorTag;
};


/**
 * General implementation class.
 *
 * Spezialize it with your derived tag to provide your own implementation of an successor state generator.
*/
template<IsSuccessorStateGeneratorInstantiation S>
class SuccessorStateGenerator : public SuccessorStateGeneratorBase<SuccessorStateGenerator<S>> { };


}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_STATE_GENERATOR_TEMPLATE_HPP_
