#ifndef MIMIR_SEARCH_HEURISTIC_HPP_
#define MIMIR_SEARCH_HEURISTIC_HPP_

#include "grounded/state_view.hpp"
#include "lifted/state_view.hpp"
#include "type_traits.hpp"

#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"


namespace mimir
{

/**
 * Interface class
*/
template<typename Derived>
class HeuristicBase : public UncopyableMixin<HeuristicBase<Derived>> {
private:
    using C = typename TypeTraits<Derived>::ConfigType;

    HeuristicBase(Problem problem) : m_problem(problem) { }
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    Problem m_problem;

public:
    [[nodiscard]] double compute_heuristic(View<StateTag<C>> state) {
        return self().compute_heuristic_impl(state);
    }
};


/**
 * Concepts
*/
template<class Derived> 
concept Heuristic = std::derived_from<Derived, HeuristicBase<Derived>>;

}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTIC_HPP_
