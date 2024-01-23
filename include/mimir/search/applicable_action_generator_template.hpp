#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATOR_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATOR_HPP_

#include "state.hpp"
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
    using P = typename TypeTraits<Derived>::PlanningModeTag;

    ApplicableActionGeneratorBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Generate all applicable actions for a given state.
    void generate_applicable_actions(View<State<P>> state, GroundActionList& out_applicable_actions) {
        self().generate_applicable_actions_impl(state, out_applicable_actions);
    }
};


/**
 * ID class.
 *
 * Derive from it to provide your own implementation of a applicable action generator.
*/
struct ApplicableActionGeneratorBaseTag {};

template<typename DerivedTag>
concept IsApplicableActionGeneratorTag = std::derived_from<DerivedTag, ApplicableActionGeneratorBaseTag>;


/**
 * Make ApplicableActionGenerator accept a PlanningModeTag.
 * This makes the template instiation less constraint
 * and we can use the PlanningModeTag of the actual search algorithm.
*/
template<IsApplicableActionGeneratorTag A, IsPlanningModeTag P>
struct ApplicableActionGeneratorInstantiation {
    using PlanningModeTag = P;
    using ApplicableActionGeneratorTag = A;
};

template<typename T>
concept IsApplicableActionGeneratorInstantiation = requires {
    typename T::PlanningModeTag;
    typename T::ApplicableActionGeneratorTag;
};


/**
 * General implementation class.
 *
 * Spezialize it with your derived tag to provide your own implementation of an applicable action generator.
*/
template<IsApplicableActionGeneratorInstantiation A>
class ApplicableActionGenerator : public ApplicableActionGeneratorBase<ApplicableActionGenerator<A>> { };



}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATOR_HPP_
